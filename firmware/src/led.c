/**
 * led.c - LED control module
 *
 * Controls the onboard LED on PB1 for status indication.
 */

#include "led.h"

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
