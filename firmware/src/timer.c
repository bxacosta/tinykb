/**
 * timer.c - Hardware timer for timing operations
 *
 * Timer1 configuration for ~1ms tick on ATtiny85 at 16.5MHz.
 */

#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

/*
 * Timer1 in CTC mode with prescaler /128:
 * - Timer frequency: 16,500,000 / 128 = 128,906.25 Hz
 * - Ticks per ms: ~129
 * - Using OCR1C as TOP value
 */
#define TIMER1_TOP 128

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static volatile uint16_t millis_counter;

/* ISR */

ISR(TIMER1_COMPA_vect) {
    millis_counter++;
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void timer_init(void) {
    /*
     * Timer1 in CTC mode:
     * - CTC1: Clear timer on compare match with OCR1C
     * - CS13: Prescaler /128 (ATtiny85 specific)
     */
    TCCR1 = (1 << CTC1) | (1 << CS13);

    OCR1C = TIMER1_TOP;
    OCR1A = TIMER1_TOP;

    TIMSK |= (1 << OCIE1A);

    millis_counter = 0;
}

/* Time Queries */

uint16_t timer_millis(void) {
    uint16_t ms;

    cli();
    ms = millis_counter;
    sei();

    return ms;
}

bool timer_elapsed(uint16_t start, uint16_t duration) {
    return (timer_millis() - start) >= duration;
}
