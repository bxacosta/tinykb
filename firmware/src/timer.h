/**
 * timer.h - Hardware timer for timing operations
 *
 * Uses Timer1 on ATtiny85 to provide millisecond-resolution timing.
 * Does not block USB - caller is responsible for calling keyboard_poll().
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Initialization */

void timer_init(void);

/* Time queries */

uint16_t timer_millis(void);
bool timer_elapsed(uint16_t start, uint16_t duration);

#endif
