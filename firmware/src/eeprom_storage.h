/**
 * eeprom_storage.h - Script storage in EEPROM
 *
 * Handles reading/writing scripts with header validation and CRC16 integrity.
 * Header format (8 bytes): magic(2) + version(1) + flags(1) + length(2) + crc16(2)
 */

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

/* Constants */

#define STORAGE_HEADER_SIZE           8
#define STORAGE_MAX_SCRIPT            504   /* 512 - 8 */
#define STORAGE_MAGIC                 0xABCD
#define STORAGE_VERSION               0x01
#define STORAGE_INITIAL_DELAY_UNIT_MS 500

/* Types */

typedef enum {
    STORAGE_OK = 0,
    STORAGE_ERR_INVALID_ADDRESS,
    STORAGE_ERR_SIZE_EXCEEDED,
    STORAGE_ERR_CHECKSUM_MISMATCH,
    STORAGE_ERR_INVALID_MAGIC
} storage_error_t;

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Initialization */

void storage_init(void);

/* Reading */

uint8_t  storage_read_byte(uint16_t offset);
uint16_t storage_get_script_length(void);
uint16_t storage_get_initial_delay(void);

/* Validation */

bool storage_has_valid_script(void);
storage_error_t storage_verify_script(void);

/* Writing */

storage_error_t storage_write_script(const uint8_t *data, uint16_t length);
void storage_clear(void);

#endif
