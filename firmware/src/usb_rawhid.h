/**
 * usb_rawhid.h - Raw HID USB interface for programming mode
 *
 * Handles Raw HID USB communication (Vendor Usage Page 0xFF00).
 * Receives HID reports from host and dispatches to hid_protocol.
 */

#ifndef USB_RAWHID_H
#define USB_RAWHID_H

#include <stdint.h>
#include <stdbool.h>
#include "usbdrv.h"

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void rawhid_init(void);

/* USB Handlers (called by usb_dispatcher.c) */

usbMsgLen_t rawhid_handle_setup(usbRequest_t *request);
usbMsgLen_t rawhid_handle_write(uint8_t *data, uint8_t length);
uint8_t rawhid_handle_read(uint8_t *data, uint8_t length);

/* Status */

bool rawhid_has_pending_response(void);
bool rawhid_should_exit(void);

/* Activity tracking */

bool rawhid_had_activity(void);

#endif /* USB_RAWHID_H */
