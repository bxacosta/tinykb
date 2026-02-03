/**
 * main.c - Entry point for TinyKB USB keyboard firmware
 *
 * Orchestrates all modules: USB keyboard, timer, EEPROM storage, and script engine.
 * Main loop maintains USB communication and executes scripts from EEPROM.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usb_keyboard.h"
#include "usb_vendor.h"
#include "timer.h"
#include "eeprom_storage.h"
#include "script_engine.h"

/* Constants */

#define PROGRAM_WINDOW_MS 2000  /* 2 second window for programming */

/* -------------------------------------------------------------------------- */
/* Private Section                                                             */
/* -------------------------------------------------------------------------- */

/**
 * Cooperative delay that maintains USB polling.
 * Does not block USB communication.
 *
 * @param duration_ms Delay duration in milliseconds
 */
static void delay_with_poll(uint16_t duration_ms) {
    uint16_t start = timer_millis();
    while (!timer_elapsed(start, duration_ms)) {
        keyboard_poll();
    }
}

/**
 * Wait for USB host connection.
 * Keeps polling USB until host communicates with us.
 */
static void wait_for_connection(void) {
    while (!keyboard_is_connected()) {
        keyboard_poll();
    }
}

/* -------------------------------------------------------------------------- */
/* Public Section                                                              */
/* -------------------------------------------------------------------------- */

int main(void) {
    /* Initialize all modules */
    keyboard_init();  /* Also handles USB disconnect/connect cycle */
    timer_init();
    storage_init();
    engine_init();
    vendor_init();

    /* Enable global interrupts (required for Timer1 and V-USB) */
    sei();

    /* Wait for USB enumeration to complete */
    wait_for_connection();

    /* Program window - allow host to enter program mode */
    vendor_set_mode(MODE_WAITING);
    uint16_t window_start = timer_millis();
    while (!timer_elapsed(window_start, PROGRAM_WINDOW_MS)) {
        keyboard_poll();
        /* Exit window early if host enters program mode */
        if (vendor_get_mode() == MODE_PROGRAM) {
            break;
        }
    }

    /* Auto-execute script if not in program mode */
    if (vendor_get_mode() == MODE_WAITING) {
        vendor_set_mode(MODE_RUNNING);

        if (storage_has_valid_script()) {
            /* Apply initial delay from EEPROM flags (0-7.5s in 500ms units) */
            uint16_t initial_delay = storage_get_initial_delay();
            if (initial_delay > 0) {
                delay_with_poll(initial_delay);
            }

            /* Start script execution */
            engine_start();
        }
    }

    /* Main loop - runs forever */
    for (;;) {
        /* Maintain USB connection (must be called every <10ms) */
        keyboard_poll();

        /* Advance script execution only in running mode */
        if (vendor_get_mode() == MODE_RUNNING) {
            engine_tick();
        }
    }

    return 0;  /* Never reached */
}
