/**
 * usb_keyboard.h - USB HID keyboard interface
 *
 * Encapsulates V-USB keyboard communication. Provides a clean interface for
 * sending keyboard reports to the host.
 */

#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define KEYBOARD_REPORT_SIZE 8
#define KEYBOARD_MAX_KEYS    6

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void keyboard_init(void);

/* USB Maintenance */

bool keyboard_is_ready(void);
bool keyboard_is_connected(void);

/* Report Sending */

bool keyboard_send_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count);
void keyboard_release_all(void);

/* LED State */

uint8_t keyboard_get_led_state(void);

/* -------------------------------------------------------------------------- */
/* Internal Handlers (called by usb_dispatcher.c)                              */
/* -------------------------------------------------------------------------- */

#include "usbdrv.h"

usbMsgLen_t keyboard_handle_setup(usbRequest_t *request);
usbMsgLen_t keyboard_handle_write(uint8_t *data, uint8_t length);

#endif /* USB_KEYBOARD_H */
