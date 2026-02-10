/**
 * usb_core.c - USB interface for application layer
 *
 * Encapsulates V-USB initialization and polling. Application modules
 * use this instead of calling V-USB directly.
 */

#include "usb_core.h"
#include "usbdrv.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define USB_DISCONNECT_MS 300

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void usb_init(void) {
    cli();
    PORTB &= ~(_BV(USB_CFG_DMINUS_BIT) | _BV(USB_CFG_DPLUS_BIT));
    usbDeviceDisconnect();
    _delay_ms(USB_DISCONNECT_MS);
    usbDeviceConnect();
    usbInit();
    sei();
}

/* Maintenance */

void usb_poll(void) {
    usbPoll();
}
