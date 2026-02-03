/**
 * crc16.c - CRC16-CCITT implementation
 */

#include "crc16.h"

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

uint16_t crc16_update(uint16_t crc, uint8_t byte) {
    crc ^= (uint16_t)byte << 8;
    for (uint8_t i = 0; i < 8; i++) {
        crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

uint16_t crc16_calculate(const uint8_t *data, uint16_t length) {
    uint16_t crc = CRC16_INIT;
    for (uint16_t i = 0; i < length; i++) {
        crc = crc16_update(crc, data[i]);
    }
    return crc;
}
