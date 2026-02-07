/**
 * crc16.c - CRC16-CCITT calculation
 *
 * Bit-by-bit implementation optimized for small code size.
 */

#include "crc16.h"
#include "config.h"

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

uint16_t crc16_init(void) {
    return CRC16_INIT;
}

/* Calculation */

uint16_t crc16_update(uint16_t crc, uint8_t byte) {
    crc ^= (uint16_t)byte << 8;

    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ CRC16_POLY;
        } else {
            crc = crc << 1;
        }
    }

    return crc;
}

uint16_t crc16_finalize(uint16_t crc) {
    return crc;
}
