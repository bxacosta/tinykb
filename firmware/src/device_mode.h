/**
 * device_mode.h - Device mode state machine
 *
 * Manages device mode transitions between Programming and Keyboard modes.
 * Uses MCUSR/GPIOR0 watchdog reset detection for mode selection.
 *
 * Mode Detection:
 *   - Watchdog reset (WDRF in MCUSR/GPIOR0) -> Keyboard mode
 *   - Any other reset source -> Programming mode (5s timeout)
 */

#ifndef DEVICE_MODE_H
#define DEVICE_MODE_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void device_mode_init(void);
void device_mode_run(void);

/* Mode Queries */

bool device_mode_is_programming(void);
bool device_mode_is_keyboard(void);

/* Mode Transitions */

void device_mode_transition_to_keyboard(void);

#endif /* DEVICE_MODE_H */
