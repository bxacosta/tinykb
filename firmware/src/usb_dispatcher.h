/**
 * usb_dispatcher.h - V-USB callback dispatcher
 *
 * Implements V-USB callbacks and routes requests to appropriate handlers
 * based on current device mode:
 *   Programming mode -> usb_rawhid
 *   Keyboard mode -> usb_keyboard
 */

#ifndef USB_DISPATCHER_H
#define USB_DISPATCHER_H

#include <stdint.h>
#include <stdbool.h>
#include "usbdrv.h"

#endif /* USB_DISPATCHER_H */
