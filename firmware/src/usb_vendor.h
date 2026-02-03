/**
 * usb_vendor.h - USB vendor request handler
 *
 * Handles vendor-specific USB requests for script programming.
 * Implements the programming protocol defined in usb-programming-interface.md.
 */

#ifndef USB_VENDOR_H
#define USB_VENDOR_H

#include <stdint.h>
#include <stdbool.h>
#include "usbdrv.h"

/* Commands (bRequest values) */

#define CMD_PROGRAM     0x00  /* Enter program mode */
#define CMD_WRITE       0x01  /* Write data chunk */
#define CMD_COMMIT      0x02  /* Commit script to EEPROM */
#define CMD_EXECUTE     0x03  /* Execute script */
#define CMD_STATUS      0x04  /* Get device status */
#define CMD_CLEAR       0x05  /* Clear EEPROM script */

/* Commit status codes (returned by CMD_COMMIT) */

#define COMMIT_SUCCESS              0x00
#define COMMIT_ERR_CRC_MISMATCH     0x01
#define COMMIT_ERR_SIZE_INVALID     0x02
#define COMMIT_ERR_WRITE_FAILED     0x03

/* Device modes */

typedef enum {
    MODE_INITIALIZING = 0,  /* Before USB enumeration */
    MODE_WAITING,           /* Program window active, waiting for host */
    MODE_PROGRAM,           /* Programming mode, receiving data */
    MODE_RUNNING            /* Script execution mode */
} device_mode_t;

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void vendor_init(void);

/* Mode management */

device_mode_t vendor_get_mode(void);
void vendor_set_mode(device_mode_t mode);
bool vendor_is_program_mode(void);

/* -------------------------------------------------------------------------- */
/* Internal Handlers (called by usb_core.c)                                   */
/* -------------------------------------------------------------------------- */

usbMsgLen_t vendor_handle_setup(usbRequest_t *rq);
usbMsgLen_t vendor_handle_write(uint8_t *data, uint8_t len);
uint8_t vendor_handle_read(uint8_t *data, uint8_t len);

#endif
