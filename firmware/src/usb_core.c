/**
 * usb_core.c - USB core dispatcher
 *
 * Implements V-USB callbacks and delegates to appropriate handlers.
 */

#include "usb_core.h"
#include "usb_keyboard.h"
#include "usb_vendor.h"
#include <avr/io.h>

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* Track current request type for multi-packet transfers */
static uint8_t current_request_type;

/* -------------------------------------------------------------------------- */
/* V-USB Callbacks                                                            */
/* -------------------------------------------------------------------------- */

/* Called by V-USB when a SETUP packet arrives. Routes to vendor or HID handler. */
usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
    usbRequest_t *rq = (void *)data;
    current_request_type = rq->bmRequestType & USBRQ_TYPE_MASK;

    if (current_request_type == USBRQ_TYPE_VENDOR) {
        return vendor_handle_setup(rq);
    }

    if (current_request_type == USBRQ_TYPE_CLASS) {
        return keyboard_handle_setup(rq);
    }

    return 0;
}

/* Called by V-USB to receive data from host (control-out transfers). */
usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len) {
    if (current_request_type == USBRQ_TYPE_VENDOR) {
        return vendor_handle_write(data, len);
    }
    return keyboard_handle_write(data, len);
}

/* Called by V-USB to send data to host (control-in transfers). */
uchar usbFunctionRead(uchar *data, uchar len) {
    if (current_request_type == USBRQ_TYPE_VENDOR) {
        return vendor_handle_read(data, len);
    }
    return 0;
}

/* Called by USB_RESET_HOOK to calibrate internal RC oscillator for stable USB. */
void calibrateOscillator(void) {
    uchar step = 128;
    uchar trialValue = 0, optimumValue;
    int x, optimumDev;
    int targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

    /* Binary search */
    do {
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();
        if (x < targetValue) {
            trialValue += step;
        }
        step >>= 1;
    } while (step > 0);

    /* Neighborhood search for optimum */
    optimumValue = trialValue;
    optimumDev = x;

    for (OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++) {
        x = usbMeasureFrameLength() - targetValue;
        if (x < 0) x = -x;
        if (x < optimumDev) {
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }

    OSCCAL = optimumValue;
}
