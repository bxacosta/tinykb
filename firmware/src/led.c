/**
 * led.c - LED control module
 *
 * Controls the onboard LED on PB1 for status indication.
 */

#include "led.h"
#include "timer.h"

#include <avr/io.h>

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void led_init(void) {
    DDRB |= (1 << LED_PIN);
    PORTB &= ~(1 << LED_PIN);
}

/* Control */

void led_on(void) {
    PORTB |= (1 << LED_PIN);
}

void led_off(void) {
    PORTB &= ~(1 << LED_PIN);
}

void led_toggle(void) {
    PORTB ^= (1 << LED_PIN);
}

bool led_is_on(void) {
    return (PORTB & (1 << LED_PIN)) != 0;
}

/* Status Indication */

void led_blink(uint8_t count, uint16_t on_ms, uint16_t off_ms, void (*idle_callback)(void)) {
    bool initial_state = led_is_on();
    
    for (uint8_t i = 0; i < count; i++) {
        led_on();
        uint16_t start = timer_millis();
        while (!timer_elapsed(start, on_ms)) {
            if (idle_callback) {
                idle_callback();
            }
        }
        
        led_off();
        start = timer_millis();
        while (!timer_elapsed(start, off_ms)) {
            if (idle_callback) {
                idle_callback();
            }
        }
    }
    
    if (initial_state) {
        led_on();
    } else {
        led_off();
    }
}

