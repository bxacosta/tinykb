/**
 * usb_keyboard.c - USB HID keyboard interface
 */

#include "usb_keyboard.h"
#include "usbdrv.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stddef.h>

/* Constants */

#define USB_DISCONNECT_MS 250

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static uint8_t report_buffer[HID_REPORT_SIZE];
static uint8_t idle_rate = 500 / 4;   /* HID 1.11 sect 7.2.4 */
static uint8_t protocol_version = 0;  /* HID 1.11 sect 7.2.6 */
static uint8_t led_state = 0;
static bool has_communicated = false;

/* HID Report Descriptor - Boot Protocol Keyboard (63 bytes) */

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,  /* USAGE_PAGE (Generic Desktop)              */
    0x09, 0x06,  /* USAGE (Keyboard)                          */
    0xA1, 0x01,  /* COLLECTION (Application)                  */

    /* Modifier byte (8 bits) */
    0x05, 0x07,  /*   USAGE_PAGE (Keyboard/Key Codes)         */
    0x19, 0xE0,  /*   USAGE_MINIMUM (224) - Left Ctrl         */
    0x29, 0xE7,  /*   USAGE_MAXIMUM (231) - Right GUI         */
    0x15, 0x00,  /*   LOGICAL_MINIMUM (0)                     */
    0x25, 0x01,  /*   LOGICAL_MAXIMUM (1)                     */
    0x75, 0x01,  /*   REPORT_SIZE (1)                         */
    0x95, 0x08,  /*   REPORT_COUNT (8)                        */
    0x81, 0x02,  /*   INPUT (Data,Var,Abs) - Modifier byte    */

    /* Reserved byte */
    0x95, 0x01,  /*   REPORT_COUNT (1)                        */
    0x75, 0x08,  /*   REPORT_SIZE (8)                         */
    0x81, 0x03,  /*   INPUT (Cnst,Var,Abs) - Reserved byte    */

    /* LED output report (5 bits + 3 padding) */
    0x95, 0x05,  /*   REPORT_COUNT (5)                        */
    0x75, 0x01,  /*   REPORT_SIZE (1)                         */
    0x05, 0x08,  /*   USAGE_PAGE (LEDs)                       */
    0x19, 0x01,  /*   USAGE_MINIMUM (Num Lock)                */
    0x29, 0x05,  /*   USAGE_MAXIMUM (Kana)                    */
    0x91, 0x02,  /*   OUTPUT (Data,Var,Abs) - LED report      */
    0x95, 0x01,  /*   REPORT_COUNT (1)                        */
    0x75, 0x03,  /*   REPORT_SIZE (3)                         */
    0x91, 0x03,  /*   OUTPUT (Cnst,Var,Abs) - LED padding     */

    /* Key array (6 bytes) */
    0x95, 0x06,  /*   REPORT_COUNT (6)                        */
    0x75, 0x08,  /*   REPORT_SIZE (8)                         */
    0x15, 0x00,  /*   LOGICAL_MINIMUM (0)                     */
    0x25, 0x65,  /*   LOGICAL_MAXIMUM (101)                   */
    0x05, 0x07,  /*   USAGE_PAGE (Keyboard/Key Codes)         */
    0x19, 0x00,  /*   USAGE_MINIMUM (0)                       */
    0x29, 0x65,  /*   USAGE_MAXIMUM (101)                     */
    0x81, 0x00,  /*   INPUT (Data,Ary,Abs) - Key array        */

    0xC0         /* END_COLLECTION                            */
};

/* Helpers */

static void build_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count) {
    report_buffer[0] = modifiers;
    report_buffer[1] = 0x00;  /* Reserved */

    for (uint8_t i = 0; i < MAX_SIMULTANEOUS_KEYS; i++) {
        report_buffer[2 + i] = (i < key_count) ? keys[i] : 0x00;
    }
}

/* -------------------------------------------------------------------------- */
/* Internal Handlers (called by usb_core.c)                                   */
/* -------------------------------------------------------------------------- */

usbMsgLen_t keyboard_handle_setup(usbRequest_t *rq) {
    has_communicated = true;

    switch (rq->bRequest) {
        case USBRQ_HID_GET_IDLE:
            usbMsgPtr = (usbMsgPtr_t)&idle_rate;
            return 1;

        case USBRQ_HID_SET_IDLE:
            idle_rate = rq->wValue.bytes[1];
            return 0;

        case USBRQ_HID_GET_PROTOCOL:
            usbMsgPtr = (usbMsgPtr_t)&protocol_version;
            return 1;

        case USBRQ_HID_SET_PROTOCOL:
            protocol_version = rq->wValue.bytes[1];
            return 0;

        case USBRQ_HID_GET_REPORT:
            usbMsgPtr = (usbMsgPtr_t)report_buffer;
            return sizeof(report_buffer);

        case USBRQ_HID_SET_REPORT:
            if (rq->wLength.word == 1) {
                return USB_NO_MSG;  /* Call keyboard_handle_write */
            }
            return 0;

        default:
            return 0;
    }
}

usbMsgLen_t keyboard_handle_write(uint8_t *data, uint8_t len) {
    if (len > 0) {
        led_state = data[0];
    }
    return 1;
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Initialization */

void keyboard_init(void) {
    cli();

    /* Force USB re-enumeration by simulating disconnect */
    PORTB &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT));
    usbDeviceDisconnect();
    _delay_ms(USB_DISCONNECT_MS);
    usbDeviceConnect();

    usbInit();

    /* Clear report buffer */
    for (uint8_t i = 0; i < HID_REPORT_SIZE; i++) {
        report_buffer[i] = 0;
    }

    sei();
}

/* USB maintenance */

void keyboard_poll(void) {
    usbPoll();
}

bool keyboard_is_ready(void) {
    return usbInterruptIsReady();
}

/* Report sending */

bool keyboard_send_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count) {
    if (!usbInterruptIsReady()) {
        return false;
    }

    if (key_count > MAX_SIMULTANEOUS_KEYS) {
        key_count = MAX_SIMULTANEOUS_KEYS;
    }

    build_report(modifiers, keys, key_count);
    usbSetInterrupt((uchar *)report_buffer, sizeof(report_buffer));

    return true;
}

void keyboard_release_all(void) {
    /* Wait until ready, then send empty report */
    while (!usbInterruptIsReady()) {
        usbPoll();
    }

    build_report(0, NULL, 0);
    usbSetInterrupt((uchar *)report_buffer, sizeof(report_buffer));
}

/* Status */

bool keyboard_is_connected(void) {
    return has_communicated;
}

uint8_t keyboard_get_led_state(void) {
    return led_state;
}
