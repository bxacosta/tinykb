/**
 * crc16.h - CRC16-CCITT calculation
 *
 * Used by EEPROM storage for script integrity verification.
 * Polynomial: 0x1021 (CCITT), Initial value: 0xFFFF
 */

#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

uint16_t crc16_init(void);

/* Calculation */

uint16_t crc16_update(uint16_t crc, uint8_t byte);
uint16_t crc16_finalize(uint16_t crc);

#endif /* CRC16_H */
