/**
 * usb_keyboard.h - USB HID keyboard interface
 *
 * Encapsulates all V-USB communication. Provides a clean interface for
 * sending keyboard reports to the host.
 */

#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

/* Constants */

#define HID_REPORT_SIZE       8
#define MAX_SIMULTANEOUS_KEYS 6

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Initialization */

void keyboard_init(void);

/* USB maintenance */

void keyboard_poll(void);
bool keyboard_is_ready(void);

/* Report sending */

bool keyboard_send_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count);
void keyboard_release_all(void);

/* Status */

bool keyboard_is_connected(void);
uint8_t keyboard_get_led_state(void);

/* -------------------------------------------------------------------------- */
/* Internal Handlers (called by usb_core.c)                                   */
/* -------------------------------------------------------------------------- */

#include "usbdrv.h"

usbMsgLen_t keyboard_handle_setup(usbRequest_t *rq);
usbMsgLen_t keyboard_handle_write(uint8_t *data, uint8_t len);

#endif
