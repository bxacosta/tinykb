/**
 * usb_rawhid.c - Raw HID USB interface for programming mode
 *
 * Handles Raw HID USB communication (Vendor Usage Page 0xFF00).
 * Receives HID reports from host and dispatches to hid_protocol.
 */

#include "usb_rawhid.h"
#include "hid_protocol.h"
#include "config.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static uint8_t report_buffer[PROTOCOL_REPORT_SIZE];
static uint8_t report_offset;
static uint8_t expected_length;
static bool response_pending;
static bool had_activity;

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void rawhid_init(void) {
    protocol_init();
    report_offset = 0;
    expected_length = 0;
    response_pending = false;
    had_activity = false;
}

/* USB Handlers */

usbMsgLen_t rawhid_handle_setup(usbRequest_t *request) {
    had_activity = true;

    switch (request->bRequest) {
        case USBRQ_HID_GET_REPORT:
            /* Host wants to read the response */
            if (response_pending) {
                const protocol_response_t *resp = protocol_get_response();
                uint8_t len = protocol_get_response_length();

                /* Copy response to return buffer */
                memcpy(report_buffer, resp, len);
                /* Zero-pad the rest */
                if (len < PROTOCOL_REPORT_SIZE) {
                    memset(report_buffer + len, 0, PROTOCOL_REPORT_SIZE - len);
                }

                usbMsgPtr = (usbMsgPtr_t)report_buffer;
                response_pending = false;
                return PROTOCOL_REPORT_SIZE;
            }
            return 0;

        case USBRQ_HID_SET_REPORT:
            /* Host wants to send a command report */
            report_offset = 0;
            expected_length = request->wLength.word;
            if (expected_length > PROTOCOL_REPORT_SIZE) {
                expected_length = PROTOCOL_REPORT_SIZE;
            }
            return USB_NO_MSG;  /* Call rawhid_handle_write for data */

        default:
            return 0;
    }
}

usbMsgLen_t rawhid_handle_write(uint8_t *data, uint8_t length) {
    had_activity = true;

    /* Accumulate incoming data */
    for (uint8_t i = 0; i < length && report_offset < expected_length; i++) {
        report_buffer[report_offset++] = data[i];
    }

    /* When complete, process the command */
    if (report_offset >= expected_length) {
        protocol_process_report(report_buffer, expected_length);
        response_pending = (protocol_get_response_length() > 0);
        report_offset = 0;
        return 1;  /* Finished receiving */
    }

    return 0;  /* More data expected */
}

uint8_t rawhid_handle_read(uint8_t *data, uint8_t length) {
    /* Copy response data for GET_REPORT */
    const protocol_response_t *resp = protocol_get_response();
    uint8_t resp_len = protocol_get_response_length();

    uint8_t to_copy = (length < resp_len) ? length : resp_len;
    memcpy(data, resp, to_copy);

    /* Zero-pad if requested length is greater */
    if (length > to_copy) {
        memset(data + to_copy, 0, length - to_copy);
    }

    response_pending = false;
    return length;
}

/* Status */

bool rawhid_has_pending_response(void) {
    return response_pending;
}

bool rawhid_should_exit(void) {
    return protocol_exit_requested();
}

/* Activity tracking */

bool rawhid_had_activity(void) {
    return had_activity;
}
