/**
 * hid_protocol.h - HID report protocol for programming mode
 *
 * Implements the command protocol (WRITE, READ, APPEND, COMMIT, etc.)
 * for programming scripts via WebHID.
 *
 * Protocol details: firmware/spec/hid-report-protocol.md
 */

#ifndef HID_PROTOCOL_H
#define HID_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

/* Commands */
#define PROTOCOL_CMD_WRITE   0x01   /* Stateless write to any address */
#define PROTOCOL_CMD_READ    0x02   /* Stateless read from any address */
#define PROTOCOL_CMD_APPEND  0x03   /* Stateful sequential write with CRC */
#define PROTOCOL_CMD_RESET   0x04   /* Reset state variables */
#define PROTOCOL_CMD_COMMIT  0x05   /* Validate CRC and write header */
#define PROTOCOL_CMD_STATUS  0x06   /* Get device state */
#define PROTOCOL_CMD_EXIT    0x07   /* Transition to keyboard mode */

/* COMMIT Options (byte 1) */
#define PROTOCOL_OPT_CRC_FROM_EEPROM  0x01  /* Bit 0: read EEPROM to calculate CRC */

/* Status Codes */
#define PROTOCOL_STATUS_OK              0x00
#define PROTOCOL_STATUS_INVALID_COMMAND 0x01
#define PROTOCOL_STATUS_INVALID_ADDRESS 0x02
#define PROTOCOL_STATUS_INVALID_LENGTH  0x03
#define PROTOCOL_STATUS_CRC_MISMATCH    0x04

/* -------------------------------------------------------------------------- */
/* Types                                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t  status;
    uint16_t result1;
    uint16_t result2;
    uint8_t  data[PROTOCOL_MAX_READ_DATA];
} protocol_response_t;

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void protocol_init(void);

/* Command Processing */

void protocol_process_report(const uint8_t *report, uint8_t length);

/* Response Access */

const protocol_response_t* protocol_get_response(void);
uint8_t protocol_get_response_length(void);
bool protocol_exit_requested(void);

#endif /* HID_PROTOCOL_H */
