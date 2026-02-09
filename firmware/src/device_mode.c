/**
 * device_mode.c - Device mode state machine
 *
 * Handles mode detection at startup, USB initialization, and runs the
 * appropriate mode loop (programming or keyboard).
 */

#include "device_mode.h"
#include "eeprom_storage.h"
#include "usb_keyboard.h"
#include "usb_rawhid.h"
#include "script_engine.h"
#include "timer.h"
#include "led.h"
#include "usbdrv.h"

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define USB_DISCONNECT_MS       300
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

/* USB Initialization */

static void init_usb(void) {
    cli();
    PORTB &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT));
    usbDeviceDisconnect();
    _delay_ms(USB_DISCONNECT_MS);
    usbDeviceConnect();
    usbInit();
    sei();
}

/* Mode Loops */

static void run_programming_loop(void) {
    init_usb();
    rawhid_init();

    uint16_t timeout_start = timer_millis();

    for (;;) {
        usbPoll();

        if (rawhid_should_exit()) {
            device_mode_transition_to_keyboard();
        }

        if (!rawhid_had_activity() && timer_elapsed(timeout_start, PROGRAMMING_TIMEOUT_MS)) {
            device_mode_transition_to_keyboard();
        }
    }
}

static void run_keyboard_loop(void) {
    init_usb();
    keyboard_init();
    engine_init();

    /* Wait for USB enumeration */
    while (!keyboard_is_connected()) {
        keyboard_poll();
    }

    /* Execute script if valid */
    if (storage_has_valid_script()) {
        uint16_t initial_delay = storage_get_initial_delay();
        if (initial_delay > 0) {
            uint16_t start = timer_millis();
            while (!timer_elapsed(start, initial_delay)) {
                keyboard_poll();
            }
        }
        engine_start();
    }

    for (;;) {
        keyboard_poll();
        engine_tick();
    }
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void device_mode_init(void) {
    current_mode = determine_initial_mode();

    if (current_mode == DEVICE_MODE_PROGRAMMING) {
        led_on();
    } else {
        led_off();
    }
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
