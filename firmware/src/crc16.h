/**
 * crc16.h - CRC16-CCITT utility
 *
 * Shared CRC16 implementation used by EEPROM storage and USB vendor interface.
 * Polynomial: 0x1021 (CCITT), Initial value: 0xFFFF
 */

#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

/* Constants */

#define CRC16_INIT 0xFFFF

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/**
 * Update CRC16 with a single byte.
 *
 * @param crc Current CRC value
 * @param byte Byte to process
 * @return Updated CRC value
 */
uint16_t crc16_update(uint16_t crc, uint8_t byte);

/**
 * Calculate CRC16 over a data buffer.
 *
 * @param data Pointer to data buffer
 * @param length Number of bytes to process
 * @return Calculated CRC16 value
 */
uint16_t crc16_calculate(const uint8_t *data, uint16_t length);

#endif
