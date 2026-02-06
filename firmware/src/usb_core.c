/**
 * usb_core.c - USB core dispatcher
 *
 * Implements V-USB callbacks and delegates to appropriate handlers.
 */

#include "usb_core.h"
#include "usb_keyboard.h"
#include <avr/io.h>

/* ========================================================================== */
/* V-USB Callbacks                                                            */
/* ========================================================================== */

usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
    usbRequest_t *request = (void *)data;

    if ((request->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        return keyboard_handle_setup(request);
    }

    return 0;
}

usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len) {
    return keyboard_handle_write(data, len);
}

uchar usbFunctionRead(uchar *data, uchar len) {
    (void)data;
    (void)len;
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
