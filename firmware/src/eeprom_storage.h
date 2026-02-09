/**
 * eeprom_storage.h - Script storage in EEPROM
 *
 * Handles reading/writing scripts with header validation and CRC16 integrity.
 *
 * Header format (8 bytes):
 *   version(1) + flags(1) + delay(2) + length(2) + crc16(2)
 *
 * EEPROM layout:
 *   [0x000 - 0x007] Header (8 bytes)
 *   [0x008 - 0x1FF] Script data (504 bytes max)
 */

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void storage_init(void);

/* Reading */

uint8_t storage_read_byte(uint16_t offset);
void storage_read_bytes(uint16_t offset, uint8_t *buffer, uint8_t length);
uint16_t storage_get_script_length(void);
uint16_t storage_get_initial_delay(void);

/* Writing */

void storage_write_byte(uint16_t offset, uint8_t value);
void storage_write_bytes(uint16_t offset, const uint8_t *data, uint8_t length);
void storage_write_header(uint8_t version, uint8_t flags, uint16_t delay, uint16_t length, uint16_t crc);
void storage_invalidate_script(void);

/* Validation */

bool storage_has_valid_script(void);
bool storage_verify_crc(uint16_t length, uint16_t expected_crc);

#endif /* EEPROM_STORAGE_H */
