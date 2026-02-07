/**
 * usb_core.c - V-USB core dispatcher
 *
 * Implements V-USB callbacks and routes requests to appropriate handlers
 * based on current device mode:
 *   Programming mode -> usb_rawhid
 *   Keyboard mode -> usb_keyboard
 */

#include "usb_core.h"
#include "usb_descriptors.h"
#include "usb_keyboard.h"
#include "usb_rawhid.h"
#include "device_mode.h"

/* -------------------------------------------------------------------------- */
/* V-USB Callbacks                                                            */
/* -------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
    usbRequest_t *request = (void *)data;

    if ((request->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        if (device_mode_is_keyboard()) {
            return keyboard_handle_setup(request);
        } else {
            return rawhid_handle_setup(request);
        }
    }

    return 0;
}

usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len) {
    if (device_mode_is_keyboard()) {
        return keyboard_handle_write(data, len);
    } else {
        return rawhid_handle_write(data, len);
    }
}

uchar usbFunctionRead(uchar *data, uchar len) {
    if (device_mode_is_keyboard()) {
        return 0;
    } else {
        return rawhid_handle_read(data, len);
    }
}

usbMsgLen_t usbFunctionDescriptor(struct usbRequest *request) {
    uint8_t descriptor_type = request->wValue.bytes[1];

    switch (descriptor_type) {
        case DESCRIPTOR_TYPE_CONFIGURATION:
            return descriptors_get_configuration();

        case DESCRIPTOR_TYPE_HID:
            return descriptors_get_hid();

        case DESCRIPTOR_TYPE_HID_REPORT:
            return descriptors_get_hid_report();

        default:
            return 0;
    }
}
