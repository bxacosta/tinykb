/**
 * usb_keyboard.c - USB HID keyboard interface
 *
 * Handles Boot Protocol HID keyboard communication.
 * Report descriptor is provided dynamically by usb_descriptors module.
 * USB connection init is handled by device_mode module.
 */

#include "usb_keyboard.h"

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static uint8_t report_buffer[KEYBOARD_REPORT_SIZE];
static uint8_t idle_rate;
static uint8_t protocol_version;
static uint8_t led_state;
static bool has_communicated;

/* Helpers */

static void build_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count) {
    report_buffer[0] = modifiers;
    report_buffer[1] = 0x00;

    for (uint8_t i = 0; i < KEYBOARD_MAX_KEYS; i++) {
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
                return USB_NO_MSG;
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

/* Lifecycle */

void keyboard_init(void) {
    for (uint8_t i = 0; i < KEYBOARD_REPORT_SIZE; i++) {
        report_buffer[i] = 0;
    }
    idle_rate = 500 / 4;
    protocol_version = 0;
    led_state = 0;
    has_communicated = false;
}

/* USB Maintenance */

void keyboard_poll(void) {
    usbPoll();
}

bool keyboard_is_ready(void) {
    return usbInterruptIsReady();
}

/* Report Sending */

bool keyboard_send_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count) {
    if (!usbInterruptIsReady()) {
        return false;
    }

    if (key_count > KEYBOARD_MAX_KEYS) {
        key_count = KEYBOARD_MAX_KEYS;
    }

    build_report(modifiers, keys, key_count);
    usbSetInterrupt((uchar *)report_buffer, sizeof(report_buffer));

    return true;
}

void keyboard_release_all(void) {
    while (!usbInterruptIsReady()) {
        usbPoll();
    }

    for (uint8_t i = 0; i < KEYBOARD_REPORT_SIZE; i++) {
        report_buffer[i] = 0;
    }
    usbSetInterrupt((uchar *)report_buffer, sizeof(report_buffer));
}

/* Status */

bool keyboard_is_connected(void) {
    return has_communicated;
}

uint8_t keyboard_get_led_state(void) {
    return led_state;
}
