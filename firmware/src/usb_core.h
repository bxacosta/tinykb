/**
 * usb_core.h - USB interface for application layer
 *
 * Provides USB lifecycle and polling functions. Encapsulates V-USB
 * so application code does not depend on the underlying USB library.
 */

#ifndef USB_CORE_H
#define USB_CORE_H

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void usb_init(void);

/* Maintenance */

void usb_poll(void);

#endif /* USB_CORE_H */
