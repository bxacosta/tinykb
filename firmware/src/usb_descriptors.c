/**
 * usb_descriptors.c - Dynamic USB descriptors
 *
 * Provides USB descriptors based on current device mode.
 * Programming mode: Raw HID (Usage Page 0xFF00, no subclass/protocol)
 * Keyboard mode: Boot Protocol HID (Usage Page 0x01, boot keyboard)
 */

#include "usb_descriptors.h"
#include "device_mode.h"
#include "config.h"
#include <avr/pgmspace.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

/* Configuration descriptor total lengths */
#define CONFIG_TOTAL_LENGTH_KEYBOARD   34   /* 9 + 9 + 9 + 7 */
#define CONFIG_TOTAL_LENGTH_RAWHID     34   /* Same structure */

/* HID report descriptor lengths */
#define HID_REPORT_LENGTH_KEYBOARD     63
#define HID_REPORT_LENGTH_RAWHID       22

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* Configuration Descriptor - Keyboard Mode (Boot Protocol HID) */

static const PROGMEM char config_descriptor_keyboard[] = {
    /* Configuration Descriptor (9 bytes) */
    9,                                      /* bLength            */
    DESCRIPTOR_TYPE_CONFIGURATION,          /* bDescriptorType    */
    CONFIG_TOTAL_LENGTH_KEYBOARD, 0,        /* wTotalLength (LE)  */
    1,                                      /* bNumInterfaces     */
    1,                                      /* bConfigurationValue*/
    0,                                      /* iConfiguration     */
    0x80,                                   /* bmAttributes (bus) */
    50,                                     /* bMaxPower (100mA)  */

    /* Interface Descriptor (9 bytes) */
    9,                                      /* bLength            */
    DESCRIPTOR_TYPE_INTERFACE,              /* bDescriptorType    */
    0,                                      /* bInterfaceNumber   */
    0,                                      /* bAlternateSetting  */
    1,                                      /* bNumEndpoints      */
    0x03,                                   /* bInterfaceClass (HID) */
    0x01,                                   /* bInterfaceSubClass (Boot) */
    0x01,                                   /* bInterfaceProtocol (Keyboard) */
    0,                                      /* iInterface         */

    /* HID Descriptor (9 bytes) */
    9,                                      /* bLength            */
    DESCRIPTOR_TYPE_HID,                    /* bDescriptorType    */
    0x11, 0x01,                             /* bcdHID (1.11) (LE) */
    0,                                      /* bCountryCode       */
    1,                                      /* bNumDescriptors    */
    DESCRIPTOR_TYPE_HID_REPORT,             /* bDescriptorType    */
    HID_REPORT_LENGTH_KEYBOARD, 0,          /* wDescriptorLength (LE) */

    /* Endpoint Descriptor (7 bytes) */
    7,                                      /* bLength            */
    DESCRIPTOR_TYPE_ENDPOINT,               /* bDescriptorType    */
    0x81,                                   /* bEndpointAddress (IN 1) */
    0x03,                                   /* bmAttributes (Interrupt) */
    8, 0,                                   /* wMaxPacketSize (LE)*/
    10                                      /* bInterval (10ms)   */
};

/* Configuration Descriptor - Programming Mode (Raw HID) */

static const PROGMEM char config_descriptor_rawhid[] = {
    /* Configuration Descriptor (9 bytes) */
    9,                                      /* bLength            */
    DESCRIPTOR_TYPE_CONFIGURATION,          /* bDescriptorType    */
    CONFIG_TOTAL_LENGTH_RAWHID, 0,          /* wTotalLength (LE)  */
    1,                                      /* bNumInterfaces     */
    1,                                      /* bConfigurationValue*/
    0,                                      /* iConfiguration     */
    0x80,                                   /* bmAttributes (bus) */
    50,                                     /* bMaxPower (100mA)  */

    /* Interface Descriptor (9 bytes) */
    9,                                      /* bLength            */
    DESCRIPTOR_TYPE_INTERFACE,              /* bDescriptorType    */
    0,                                      /* bInterfaceNumber   */
    0,                                      /* bAlternateSetting  */
    1,                                      /* bNumEndpoints      */
    0x03,                                   /* bInterfaceClass (HID) */
    0x00,                                   /* bInterfaceSubClass (None) */
    0x00,                                   /* bInterfaceProtocol (None) */
    0,                                      /* iInterface         */

    /* HID Descriptor (9 bytes) */
    9,                                      /* bLength            */
    DESCRIPTOR_TYPE_HID,                    /* bDescriptorType    */
    0x11, 0x01,                             /* bcdHID (1.11) (LE) */
    0,                                      /* bCountryCode       */
    1,                                      /* bNumDescriptors    */
    DESCRIPTOR_TYPE_HID_REPORT,             /* bDescriptorType    */
    HID_REPORT_LENGTH_RAWHID, 0,            /* wDescriptorLength (LE) */

    /* Endpoint Descriptor (7 bytes) */
    7,                                      /* bLength            */
    DESCRIPTOR_TYPE_ENDPOINT,               /* bDescriptorType    */
    0x81,                                   /* bEndpointAddress (IN 1) */
    0x03,                                   /* bmAttributes (Interrupt) */
    PROTOCOL_REPORT_SIZE, 0,                /* wMaxPacketSize (LE)*/
    10                                      /* bInterval (10ms)   */
};

/* HID Report Descriptor - Keyboard Mode (Boot Protocol, 63 bytes) */

static const PROGMEM char hid_report_keyboard[] = {
    0x05, 0x01,         /* USAGE_PAGE (Generic Desktop)              */
    0x09, 0x06,         /* USAGE (Keyboard)                          */
    0xA1, 0x01,         /* COLLECTION (Application)                  */

    /* Modifier byte (8 bits) */
    0x05, 0x07,         /*   USAGE_PAGE (Keyboard/Key Codes)         */
    0x19, 0xE0,         /*   USAGE_MINIMUM (224) - Left Ctrl         */
    0x29, 0xE7,         /*   USAGE_MAXIMUM (231) - Right GUI         */
    0x15, 0x00,         /*   LOGICAL_MINIMUM (0)                     */
    0x25, 0x01,         /*   LOGICAL_MAXIMUM (1)                     */
    0x75, 0x01,         /*   REPORT_SIZE (1)                         */
    0x95, 0x08,         /*   REPORT_COUNT (8)                        */
    0x81, 0x02,         /*   INPUT (Data,Var,Abs) - Modifier byte    */

    /* Reserved byte */
    0x95, 0x01,         /*   REPORT_COUNT (1)                        */
    0x75, 0x08,         /*   REPORT_SIZE (8)                         */
    0x81, 0x03,         /*   INPUT (Cnst,Var,Abs) - Reserved byte    */

    /* LED output report (5 bits + 3 padding) */
    0x95, 0x05,         /*   REPORT_COUNT (5)                        */
    0x75, 0x01,         /*   REPORT_SIZE (1)                         */
    0x05, 0x08,         /*   USAGE_PAGE (LEDs)                       */
    0x19, 0x01,         /*   USAGE_MINIMUM (Num Lock)                */
    0x29, 0x05,         /*   USAGE_MAXIMUM (Kana)                    */
    0x91, 0x02,         /*   OUTPUT (Data,Var,Abs) - LED report      */
    0x95, 0x01,         /*   REPORT_COUNT (1)                        */
    0x75, 0x03,         /*   REPORT_SIZE (3)                         */
    0x91, 0x03,         /*   OUTPUT (Cnst,Var,Abs) - LED padding     */

    /* Key array (6 bytes) */
    0x95, 0x06,         /*   REPORT_COUNT (6)                        */
    0x75, 0x08,         /*   REPORT_SIZE (8)                         */
    0x15, 0x00,         /*   LOGICAL_MINIMUM (0)                     */
    0x25, 0x65,         /*   LOGICAL_MAXIMUM (101)                   */
    0x05, 0x07,         /*   USAGE_PAGE (Keyboard/Key Codes)         */
    0x19, 0x00,         /*   USAGE_MINIMUM (0)                       */
    0x29, 0x65,         /*   USAGE_MAXIMUM (101)                     */
    0x81, 0x00,         /*   INPUT (Data,Ary,Abs) - Key array        */

    0xC0                /* END_COLLECTION                            */
};

/* HID Report Descriptor - Programming Mode (Raw HID, 22 bytes) */

static const PROGMEM char hid_report_rawhid[] = {
    0x06, 0x00, 0xFF,   /* USAGE_PAGE (Vendor Defined 0xFF00)        */
    0x09, 0x01,         /* USAGE (Vendor Usage 1)                    */
    0xA1, 0x01,         /* COLLECTION (Application)                  */

    0x15, 0x00,         /*   LOGICAL_MINIMUM (0)                     */
    0x26, 0xFF, 0x00,   /*   LOGICAL_MAXIMUM (255)                   */
    0x75, 0x08,         /*   REPORT_SIZE (8)                         */
    0x95, PROTOCOL_REPORT_SIZE, /*   REPORT_COUNT (32)               */
    0x09, 0x01,         /*   USAGE (Vendor Usage 1)                  */
    0x81, 0x02,         /*   INPUT (Data,Var,Abs)                    */
    0x09, 0x01,         /*   USAGE (Vendor Usage 1)                  */
    0x91, 0x02,         /*   OUTPUT (Data,Var,Abs)                   */

    0xC0                /* END_COLLECTION                            */
};

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

usbMsgLen_t descriptors_get_configuration(void) {
    if (device_mode_is_keyboard()) {
        usbMsgPtr = (usbMsgPtr_t)config_descriptor_keyboard;
        return sizeof(config_descriptor_keyboard);
    }

    usbMsgPtr = (usbMsgPtr_t)config_descriptor_rawhid;
    return sizeof(config_descriptor_rawhid);
}

usbMsgLen_t descriptors_get_hid(void) {
    /* HID descriptor is embedded in configuration descriptor at offset 18 */
    if (device_mode_is_keyboard()) {
        usbMsgPtr = (usbMsgPtr_t)(config_descriptor_keyboard + 18);
        return 9;
    }

    usbMsgPtr = (usbMsgPtr_t)(config_descriptor_rawhid + 18);
    return 9;
}

usbMsgLen_t descriptors_get_hid_report(void) {
    if (device_mode_is_keyboard()) {
        usbMsgPtr = (usbMsgPtr_t)hid_report_keyboard;
        return sizeof(hid_report_keyboard);
    }

    usbMsgPtr = (usbMsgPtr_t)hid_report_rawhid;
    return sizeof(hid_report_rawhid);
}
