/**
 * usbconfig.h - V-USB configuration for TinyKB
 *
 * This configuration enables dynamic USB descriptors for dual-mode operation:
 * - Programming mode: Raw HID (WebHID compatible)
 * - Keyboard mode: Boot Protocol HID
 *
 * Based on V-USB configuration template.
 * Modified for ATtiny85/Digispark USB Keyboard.
 */

#ifndef USBCONFIG_H
#define USBCONFIG_H

/* -------------------------------------------------------------------------- */
/* Hardware Configuration                                                     */
/* -------------------------------------------------------------------------- */

#define USB_CFG_IOPORTNAME      B
#define USB_CFG_DMINUS_BIT      3
#define USB_CFG_DPLUS_BIT       4
#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)
#define USB_CFG_CHECK_CRC       0

/* -------------------------------------------------------------------------- */
/* Functional Range                                                           */
/* -------------------------------------------------------------------------- */

#define USB_CFG_HAVE_INTRIN_ENDPOINT        1
#define USB_CFG_HAVE_INTRIN_ENDPOINT3       0
#define USB_CFG_EP3_NUMBER                  3
#define USB_CFG_IMPLEMENT_HALT              0
#define USB_CFG_SUPPRESS_INTR_CODE          0
#define USB_CFG_INTR_POLL_INTERVAL          10
#define USB_CFG_IS_SELF_POWERED             0
#define USB_CFG_MAX_BUS_POWER               100
#define USB_CFG_IMPLEMENT_FN_WRITE          1
#define USB_CFG_IMPLEMENT_FN_READ           1
#define USB_CFG_IMPLEMENT_FN_WRITEOUT       0
#define USB_CFG_HAVE_FLOWCONTROL            0
#define USB_CFG_DRIVER_FLASH_PAGE           0
#define USB_CFG_LONG_TRANSFERS              0
#define USB_COUNT_SOF                       0
#define USB_CFG_CHECK_DATA_TOGGLING         0
#define USB_CFG_HAVE_MEASURE_FRAME_LENGTH   1
#define USB_USE_FAST_CRC                    0

/* -------------------------------------------------------------------------- */
/* Oscillator Calibration                                                     */
/* -------------------------------------------------------------------------- */

#ifndef __ASSEMBLER__
#include <avr/interrupt.h>
extern void calibrate_oscillator(void);
#endif
#define USB_RESET_HOOK(resetStarts)  if(!resetStarts){cli(); calibrate_oscillator(); sei();}

/* -------------------------------------------------------------------------- */
/* Device Description                                                         */
/* -------------------------------------------------------------------------- */

#define USB_CFG_VENDOR_ID       0xc0, 0x16  /* 0x16C0 = voti.nl       */
#define USB_CFG_DEVICE_ID       0xdb, 0x27  /* 0x27DB = shared HID    */
#define USB_CFG_DEVICE_VERSION  0x00, 0x01  /* v1.00                  */

#define USB_CFG_VENDOR_NAME     'T', 'i', 'n', 'y', 'K', 'B'
#define USB_CFG_VENDOR_NAME_LEN 6

#define USB_CFG_DEVICE_NAME     'T', 'i', 'n', 'y', 'K', 'B'
#define USB_CFG_DEVICE_NAME_LEN 6

/* Device/Interface class set at interface level for HID */
#define USB_CFG_DEVICE_CLASS        0
#define USB_CFG_DEVICE_SUBCLASS     0

/*
 * Interface class/subclass/protocol are handled dynamically via
 * usb_descriptors.c based on device mode:
 *   Programming mode: Class=0x03, SubClass=0x00, Protocol=0x00
 *   Keyboard mode:    Class=0x03, SubClass=0x01, Protocol=0x01
 *
 * These defaults are for the device descriptor (not used for HID).
 */
#define USB_CFG_INTERFACE_CLASS     0x03    /* HID */
#define USB_CFG_INTERFACE_SUBCLASS  0x00    /* Dynamic via descriptors */
#define USB_CFG_INTERFACE_PROTOCOL  0x00    /* Dynamic via descriptors */

/* -------------------------------------------------------------------------- */
/* Dynamic Descriptors                                                        */
/* -------------------------------------------------------------------------- */

/*
 * Enable dynamic descriptor generation via usbFunctionDescriptor().
 * This allows switching descriptors based on device mode at runtime.
 */
#define USB_CFG_DESCR_PROPS_DEVICE                  0
#define USB_CFG_DESCR_PROPS_CONFIGURATION           USB_PROP_IS_DYNAMIC
#define USB_CFG_DESCR_PROPS_STRINGS                 0
#define USB_CFG_DESCR_PROPS_STRING_0                0
#define USB_CFG_DESCR_PROPS_STRING_VENDOR           0
#define USB_CFG_DESCR_PROPS_STRING_PRODUCT          0
#define USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER    0
#define USB_CFG_DESCR_PROPS_HID                     USB_PROP_IS_DYNAMIC
#define USB_CFG_DESCR_PROPS_HID_REPORT              USB_PROP_IS_DYNAMIC
#define USB_CFG_DESCR_PROPS_UNKNOWN                 0

/* usbMsgPtr type for flash pointers */
#define usbMsgPtr_t unsigned short

/* -------------------------------------------------------------------------- */
/* ATtiny85 Pin Change Interrupt                                              */
/* -------------------------------------------------------------------------- */

#define USB_INTR_CFG            PCMSK
#define USB_INTR_CFG_SET        (1 << USB_CFG_DPLUS_BIT)
#define USB_INTR_CFG_CLR        0
#define USB_INTR_ENABLE         GIMSK
#define USB_INTR_ENABLE_BIT     PCIE
#define USB_INTR_PENDING        GIFR
#define USB_INTR_PENDING_BIT    PCIF
#define USB_INTR_VECTOR         PCINT0_vect

#endif /* USBCONFIG_H */
