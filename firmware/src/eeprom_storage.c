/**
 * eeprom_storage.c - Script storage in EEPROM
 */

#include "eeprom_storage.h"
#include "crc16.h"
#include <avr/eeprom.h>

/* Constants */

/* Header offsets (all multi-byte fields are little-endian) */
#define OFFSET_MAGIC    0   /* 2 bytes */
#define OFFSET_VERSION  2   /* 1 byte  */
#define OFFSET_FLAGS    3   /* 1 byte  */
#define OFFSET_LENGTH   4   /* 2 bytes */
#define OFFSET_CRC      6   /* 2 bytes */

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static struct {
    uint16_t length;
    uint16_t crc;
    uint8_t  flags;
    bool     valid;
} cache;

/* Helpers */

static uint16_t read_u16(uint16_t offset) {
    return eeprom_read_byte((const uint8_t *)offset) |
           ((uint16_t)eeprom_read_byte((const uint8_t *)(offset + 1)) << 8);
}

static void write_u16(uint16_t offset, uint16_t value) {
    eeprom_write_byte((uint8_t *)offset, value & 0xFF);
    eeprom_write_byte((uint8_t *)(offset + 1), (value >> 8) & 0xFF);
}

static uint16_t calculate_script_crc(uint16_t length) {
    uint16_t crc = CRC16_INIT;
    for (uint16_t i = 0; i < length; i++) {
        crc = crc16_update(crc, eeprom_read_byte((const uint8_t *)(STORAGE_HEADER_SIZE + i)));
    }
    return crc;
}

static bool validate_header(void) {
    if (read_u16(OFFSET_MAGIC) != STORAGE_MAGIC) {
        cache.valid = false;
        return false;
    }

    cache.flags = eeprom_read_byte((const uint8_t *)OFFSET_FLAGS);
    cache.length = read_u16(OFFSET_LENGTH);
    cache.crc = read_u16(OFFSET_CRC);

    cache.valid = (cache.length > 0) && (cache.length <= STORAGE_MAX_SCRIPT);
    return cache.valid;
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Initialization */

void storage_init(void) {
    validate_header();
}

/* Reading */

uint8_t storage_read_byte(uint16_t offset) {
    return eeprom_read_byte((const uint8_t *)(STORAGE_HEADER_SIZE + offset));
}

uint16_t storage_get_script_length(void) {
    return cache.valid ? cache.length : 0;
}

uint16_t storage_get_initial_delay(void) {
    if (!cache.valid) return 0;
    return (uint16_t)(cache.flags & 0x0F) * STORAGE_INITIAL_DELAY_UNIT_MS;
}

/* Validation */

bool storage_has_valid_script(void) {
    return cache.valid;
}

storage_error_t storage_verify_script(void) {
    if (read_u16(OFFSET_MAGIC) != STORAGE_MAGIC) {
        return STORAGE_ERR_INVALID_MAGIC;
    }

    uint16_t length = read_u16(OFFSET_LENGTH);
    if (length == 0 || length > STORAGE_MAX_SCRIPT) {
        return STORAGE_ERR_SIZE_EXCEEDED;
    }

    if (read_u16(OFFSET_CRC) != calculate_script_crc(length)) {
        return STORAGE_ERR_CHECKSUM_MISMATCH;
    }

    return STORAGE_OK;
}

/* Writing */

storage_error_t storage_write_script(const uint8_t *data, uint16_t length) {
    if (length == 0 || length > STORAGE_MAX_SCRIPT) {
        return STORAGE_ERR_SIZE_EXCEEDED;
    }

    for (uint16_t i = 0; i < length; i++) {
        eeprom_write_byte((uint8_t *)(STORAGE_HEADER_SIZE + i), data[i]);
    }
    uint16_t crc = crc16_calculate(data, length);

    write_u16(OFFSET_MAGIC, STORAGE_MAGIC);
    eeprom_write_byte((uint8_t *)OFFSET_VERSION, STORAGE_VERSION);
    eeprom_write_byte((uint8_t *)OFFSET_FLAGS, 0x02);
    write_u16(OFFSET_LENGTH, length);
    write_u16(OFFSET_CRC, crc);

    cache.length = length;
    cache.crc = crc;
    cache.flags = 0x02;
    cache.valid = true;

    return STORAGE_OK;
}

void storage_clear(void) {
    write_u16(OFFSET_MAGIC, 0xFFFF);
    cache.valid = false;
    cache.length = 0;
}
