/**
 * usb_descriptors.h - Dynamic USB descriptors
 *
 * Provides USB descriptors based on current device mode.
 * Programming mode: Raw HID (Usage Page 0xFF00)
 * Keyboard mode: Boot Protocol HID (Usage Page 0x01)
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>
#include <stdbool.h>
#include "usbdrv.h"

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

/* Descriptor types */
#define DESCRIPTOR_TYPE_DEVICE        0x01
#define DESCRIPTOR_TYPE_CONFIGURATION 0x02
#define DESCRIPTOR_TYPE_STRING        0x03
#define DESCRIPTOR_TYPE_INTERFACE     0x04
#define DESCRIPTOR_TYPE_ENDPOINT      0x05
#define DESCRIPTOR_TYPE_HID           0x21
#define DESCRIPTOR_TYPE_HID_REPORT    0x22

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Descriptor Access (called by usb_core.c via usbFunctionDescriptor) */

usbMsgLen_t descriptors_get_configuration(void);
usbMsgLen_t descriptors_get_hid(void);
usbMsgLen_t descriptors_get_hid_report(void);

#endif /* USB_DESCRIPTORS_H */
