/**
 * main.c - Entry point for TinyKB USB keyboard firmware
 *
 * Orchestrates all modules: USB keyboard, timer, EEPROM storage, and script engine.
 * Main loop maintains USB communication and executes scripts from EEPROM.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "usb_keyboard.h"
#include "timer.h"
#include "eeprom_storage.h"
#include "script_engine.h"

/* ========================================================================== */
/* Constants                                                                  */
/* ========================================================================== */

#define LED_PIN PB1

/* ========================================================================== */
/* Private Functions                                                          */
/* ========================================================================== */

static void led_init(void) {
    DDRB |= (1 << LED_PIN);
    PORTB &= ~(1 << LED_PIN);
}

static void led_on(void) {
    PORTB |= (1 << LED_PIN);
}

static void led_off(void) {
    PORTB &= ~(1 << LED_PIN);
}

static void delay_with_poll(uint16_t duration_ms) {
    uint16_t start = timer_millis();
    while (!timer_elapsed(start, duration_ms)) {
        keyboard_poll();
    }
}

static void wait_for_connection(void) {
    while (!keyboard_is_connected()) {
        keyboard_poll();
    }
}

/* ========================================================================== */
/* Main Entry Point                                                           */
/* ========================================================================== */

int main(void) {
    /* Initialize modules */
    keyboard_init();
    timer_init();
    storage_init();
    engine_init();
    led_init();

    /* Enable global interrupts */
    sei();

    /* Wait for USB enumeration */
    wait_for_connection();

    /* Execute script if valid */
    if (storage_has_valid_script()) {
        led_on();

        uint16_t initial_delay = storage_get_initial_delay();
        if (initial_delay > 0) {
            delay_with_poll(initial_delay);
        }

        engine_start();
        led_off();
    }

    /* Main loop */
    for (;;) {
        keyboard_poll();
        engine_tick();
    }

    return 0;
}
