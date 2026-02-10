/**
 * device_mode.c - Device mode state machine
 *
 * Handles mode detection at startup, USB initialization, and runs the
 * appropriate mode loop (programming or keyboard).
 */

#include "device_mode.h"
#include "eeprom_storage.h"
#include "usb_core.h"
#include "usb_keyboard.h"
#include "usb_rawhid.h"
#include "script_engine.h"
#include "timer.h"
#include "led.h"

#include <avr/io.h>
#include <avr/wdt.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define PROGRAMMING_TIMEOUT_MS  5000

/* -------------------------------------------------------------------------- */
/* Types                                                                      */
/* -------------------------------------------------------------------------- */

typedef enum {
    DEVICE_MODE_PROGRAMMING,
    DEVICE_MODE_KEYBOARD
} device_mode_t;

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static device_mode_t current_mode;

/* Mode Detection */

static device_mode_t determine_initial_mode(void) {
    uint8_t reset_source = MCUSR;
    if (reset_source == 0) {
        reset_source = GPIOR0;
    }
    MCUSR = 0;
    wdt_disable();

    if (reset_source & _BV(WDRF)) {
        return DEVICE_MODE_KEYBOARD;
    }

    return DEVICE_MODE_PROGRAMMING;
}

static void trigger_watchdog_reset(void) {
    wdt_enable(WDTO_15MS);
    for (;;) {
        /* Wait for watchdog reset */
    }
}

/* Mode Loops */

static void run_programming_loop(void) {
    usb_init();
    rawhid_init();

    led_on();

    uint16_t timeout_start = timer_millis();

    for (;;) {
        usb_poll();

        if (rawhid_should_exit()) {
            device_mode_transition_to_keyboard();
        }

        if (!rawhid_had_activity() && timer_elapsed(timeout_start, PROGRAMMING_TIMEOUT_MS)) {
            device_mode_transition_to_keyboard();
        }
    }
}

static void run_keyboard_loop(void) {
    usb_init();
    keyboard_init();
    engine_init();

    led_off();

    while (!keyboard_is_connected()) {
        usb_poll();
    }

    led_blink(2, 80, 80, usb_poll);
    engine_start();

    for (;;) {
        usb_poll();
        engine_tick();
    }
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void device_mode_init(void) {
    current_mode = determine_initial_mode();
}

void device_mode_run(void) {
    if (current_mode == DEVICE_MODE_KEYBOARD) {
        run_keyboard_loop();
    } else {
        run_programming_loop();
    }
}

/* Mode Queries */

bool device_mode_is_programming(void) {
    return current_mode == DEVICE_MODE_PROGRAMMING;
}

bool device_mode_is_keyboard(void) {
    return current_mode == DEVICE_MODE_KEYBOARD;
}

/* Mode Transitions */

void device_mode_transition_to_keyboard(void) {
    trigger_watchdog_reset();
}
