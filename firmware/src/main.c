/**
 * main.c - Entry point for TinyKB firmware
 *
 * Pure orchestrator. Initializes modules and delegates to device_mode.
 */

#include "led.h"
#include "timer.h"
#include "eeprom_storage.h"
#include "device_mode.h"

int main(void) {
    led_init();
    timer_init();
    storage_init();
    device_mode_init();

    device_mode_run();  /* Never returns */

    return 0;
}
