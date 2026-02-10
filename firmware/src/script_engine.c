/**
 * script_engine.c - Bytecode interpreter for keyboard scripts
 */

#include "script_engine.h"
#include "eeprom_storage.h"
#include "config.h"
#include "usb_core.h"
#include "usb_keyboard.h"
#include "keycode.h"
#include "timer.h"

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static struct {
    uint16_t ptr;
    uint16_t length;
    engine_state_t state;

    uint8_t modifiers;
    uint8_t keys[KEYBOARD_MAX_KEYS];
    uint8_t key_count;

    uint16_t delay_start;
    uint16_t delay_duration;

    uint16_t repeat_start;
    uint8_t repeat_count;
    uint8_t repeat_length;
    bool in_repeat;
} engine;

/* Key management */

static bool add_key(uint8_t keycode) {
    for (uint8_t i = 0; i < engine.key_count; i++) {
        if (engine.keys[i] == keycode) {
            return true;
        }
    }

    if (engine.key_count >= KEYBOARD_MAX_KEYS) {
        return false;
    }

    engine.keys[engine.key_count++] = keycode;
    return true;
}

static void remove_key(uint8_t keycode) {
    for (uint8_t i = 0; i < engine.key_count; i++) {
        if (engine.keys[i] == keycode) {
            engine.keys[i] = engine.keys[--engine.key_count];
            return;
        }
    }
}

static void clear_all_keys(void) {
    engine.modifiers = 0;
    engine.key_count = 0;
}

/* Report sending */

static void send_report(void) {
    while (!keyboard_is_ready()) {
        usb_poll();
    }
    keyboard_send_report(engine.modifiers, engine.keys, engine.key_count);
}

/* Script reading */

static uint8_t read_byte(void) {
    if (engine.ptr >= engine.length) {
        engine.state = ENGINE_ERROR;
        return OP_END;
    }
    return storage_read_byte(STORAGE_SCRIPT_START + engine.ptr++);
}

static uint16_t read_u16(void) {
    uint8_t lo = read_byte();
    uint8_t hi = read_byte();
    return ((uint16_t)hi << 8) | lo;
}

/* Opcode handlers */

static void op_end(void) {
    clear_all_keys();
    send_report();
    engine.state = ENGINE_FINISHED;
}

static void op_delay(void) {
    engine.delay_duration = read_u16();
    engine.delay_start = timer_millis();
    engine.state = ENGINE_DELAYING;
}

static void op_key_down(void) {
    uint8_t keycode = read_byte();
    add_key(keycode);
    send_report();
}

static void op_key_up(void) {
    uint8_t keycode = read_byte();
    remove_key(keycode);
    send_report();
}

static void op_mod(void) {
    engine.modifiers = read_byte();
    send_report();
}

static void op_tap(uint8_t keycode) {
    add_key(keycode);
    send_report();
    remove_key(keycode);
    send_report();
}

static void op_tap_opcode(void) {
    uint8_t keycode = read_byte();
    op_tap(keycode);
}

static void op_repeat(void) {
    if (engine.in_repeat) {
        uint8_t count = read_byte();
        uint8_t length = read_byte();
        (void)count;
        engine.ptr += length;
        return;
    }

    engine.repeat_count = read_byte();
    engine.repeat_length = read_byte();
    engine.repeat_start = engine.ptr;
    engine.in_repeat = true;
}

static void op_combo(void) {
    uint8_t mod_mask = read_byte();
    uint8_t keycode = read_byte();

    uint8_t saved_mods = engine.modifiers;

    engine.modifiers = mod_mask;
    add_key(keycode);
    send_report();

    remove_key(keycode);
    engine.modifiers = saved_mods;
    send_report();
}

static void op_string(void) {
    uint8_t length = read_byte();

    for (uint8_t i = 0; i < length; i++) {
        char c = (char)read_byte();

        if (engine.state == ENGINE_ERROR) {
            return;
        }

        keycode_result_t result = keycode_from_ascii(c);

        if (result.keycode == 0) {
            continue;
        }

        if (result.modifiers != 0) {
            uint8_t saved_mods = engine.modifiers;
            engine.modifiers = result.modifiers;
            op_tap(result.keycode);
            engine.modifiers = saved_mods;
            send_report();
        } else {
            op_tap(result.keycode);
        }

        usb_poll();
    }
}

/* Execute one opcode */

static void execute_opcode(void) {
    uint8_t opcode = read_byte();

    if (engine.state == ENGINE_ERROR) {
        return;
    }

    switch (opcode) {
        case OP_END:      op_end();        break;
        case OP_DELAY:    op_delay();      break;
        case OP_KEY_DOWN: op_key_down();   break;
        case OP_KEY_UP:   op_key_up();     break;
        case OP_MOD:      op_mod();        break;
        case OP_TAP:      op_tap_opcode(); break;
        case OP_REPEAT:   op_repeat();     break;
        case OP_COMBO:    op_combo();      break;
        case OP_STRING:   op_string();     break;
        default:
            engine.state = ENGINE_ERROR;
            clear_all_keys();
            send_report();
            break;
    }
}

/* Check REPEAT block end */

static void check_repeat(void) {
    if (!engine.in_repeat) {
        return;
    }

    if (engine.ptr >= engine.repeat_start + engine.repeat_length) {
        engine.repeat_count--;
        if (engine.repeat_count > 0) {
            engine.ptr = engine.repeat_start;
        } else {
            engine.in_repeat = false;
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void engine_init(void) {
    engine.state = ENGINE_IDLE;
    engine.ptr = 0;
    engine.length = 0;
    engine.modifiers = 0;
    engine.key_count = 0;
    engine.in_repeat = false;
}

void engine_start(void) {
    if (!storage_has_valid_script()) {
        engine.state = ENGINE_IDLE;
        return;
    }

    uint16_t initial_delay = storage_get_initial_delay();
    if (initial_delay > 0) {
        uint16_t start = timer_millis();
        while (!timer_elapsed(start, initial_delay)) {
            usb_poll();
        }
    }

    engine.ptr = 0;
    engine.length = storage_get_script_length();
    engine.state = ENGINE_RUNNING;
    engine.modifiers = 0;
    engine.key_count = 0;
    engine.in_repeat = false;
}

void engine_stop(void) {
    clear_all_keys();
    send_report();
    engine.state = ENGINE_IDLE;
}

/* Execution */

engine_state_t engine_tick(void) {
    switch (engine.state) {
        case ENGINE_IDLE:
        case ENGINE_FINISHED:
        case ENGINE_ERROR:
            break;

        case ENGINE_RUNNING:
            execute_opcode();
            check_repeat();
            break;

        case ENGINE_DELAYING:
            if (timer_elapsed(engine.delay_start, engine.delay_duration)) {
                engine.state = ENGINE_RUNNING;
            }
            break;
    }

    return engine.state;
}

/* Status */

engine_state_t engine_get_state(void) {
    return engine.state;
}

bool engine_is_running(void) {
    return engine.state == ENGINE_RUNNING || engine.state == ENGINE_DELAYING;
}
