/**
 * oscillator.c - RC oscillator calibration for V-USB
 *
 * Uses binary search followed by neighborhood search to find
 * the optimal OSCCAL value for stable USB timing at 16.5MHz.
 */

#include "oscillator.h"

#include <avr/io.h>
#include "usbdrv.h"

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Calibration */

void calibrate_oscillator(void) {
    uint8_t step = 128;
    uint8_t trial_value = 0;
    uint8_t optimum_value;
    int16_t x;
    int16_t optimum_dev;
    int16_t target_value = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

    /* Binary search for approximate value */
    do {
        OSCCAL = trial_value + step;
        x = usbMeasureFrameLength();
        if (x < target_value) {
            trial_value += step;
        }
        step >>= 1;
    } while (step > 0);

    /* Neighborhood search for optimum */
    optimum_value = trial_value;
    optimum_dev = x;

    for (OSCCAL = trial_value - 1; OSCCAL <= trial_value + 1; OSCCAL++) {
        x = usbMeasureFrameLength() - target_value;
        if (x < 0) {
            x = -x;
        }
        if (x < optimum_dev) {
            optimum_dev = x;
            optimum_value = OSCCAL;
        }
    }

    OSCCAL = optimum_value;
}
