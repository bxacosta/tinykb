/**
 * oscillator.h - RC oscillator calibration for V-USB
 *
 * Calibrates the internal RC oscillator to achieve stable USB timing.
 * Called automatically by V-USB via USB_RESET_HOOK during USB enumeration.
 */

#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Calibration */

void calibrate_oscillator(void);

#endif /* OSCILLATOR_H */
