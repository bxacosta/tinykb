/**
 * led.h - LED control module
 *
 * Encapsulates LED hardware control for status indication.
 * Uses PB1 on ATtiny85 (Digispark onboard LED).
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define LED_PIN PB1

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void led_init(void);

/* Control */

void led_on(void);
void led_off(void);
void led_toggle(void);
bool led_is_on(void);

/* Status Indication */

void led_blink(uint8_t count, uint16_t on_ms, uint16_t off_ms);

#endif /* LED_H */
