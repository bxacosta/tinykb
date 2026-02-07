/**
 * timer.h - Hardware timer for timing operations
 *
 * Uses Timer1 on ATtiny85 to provide millisecond-resolution timing.
 * Non-blocking: caller is responsible for calling usbPoll() or keyboard_poll().
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void timer_init(void);

/* Time Queries */

uint16_t timer_millis(void);
bool timer_elapsed(uint16_t start, uint16_t duration);

#endif /* TIMER_H */
