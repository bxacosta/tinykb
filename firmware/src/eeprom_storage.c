/**
 * eeprom_storage.c - Script storage in EEPROM
 *
 * Provides low-level EEPROM access for script storage.
 * Uses eeprom_update_byte() for writes to extend EEPROM lifespan.
 */

#include "eeprom_storage.h"
#include "config.h"
#include "crc16.h"

#include <avr/eeprom.h>

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static struct {
    uint16_t length;
    uint16_t delay;
    uint8_t  flags;
    bool     valid;
} cache;

/* Helpers */

static uint16_t read_u16(uint16_t addr) {
    uint8_t low = eeprom_read_byte((const uint8_t *)addr);
    uint8_t high = eeprom_read_byte((const uint8_t *)(addr + 1));
    return (uint16_t)low | ((uint16_t)high << 8);
}

static void write_u16(uint16_t addr, uint16_t value) {
    eeprom_update_byte((uint8_t *)addr, value & 0xFF);
    eeprom_update_byte((uint8_t *)(addr + 1), (value >> 8) & 0xFF);
}

static bool validate_header(void) {
    uint8_t version = eeprom_read_byte((const uint8_t *)HEADER_OFFSET_VERSION);

    if (version != STORAGE_PAYLOAD_VERSION) {
        cache.valid = false;
        return false;
    }

    cache.flags = eeprom_read_byte((const uint8_t *)HEADER_OFFSET_FLAGS);
    cache.delay = read_u16(HEADER_OFFSET_DELAY);
    cache.length = read_u16(HEADER_OFFSET_LENGTH);

    cache.valid = (cache.length > 0) && (cache.length <= STORAGE_MAX_SCRIPT_SIZE);
    return cache.valid;
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void storage_init(void) {
    validate_header();
}

/* Reading */

uint8_t storage_read_byte(uint16_t offset) {
    if (offset >= STORAGE_MAX_SCRIPT_SIZE) {
        return 0xFF;
    }
    return eeprom_read_byte((const uint8_t *)(STORAGE_SCRIPT_START + offset));
}

void storage_read_bytes(uint16_t offset, uint8_t *buffer, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        if (offset + i >= STORAGE_MAX_SCRIPT_SIZE) {
            buffer[i] = 0xFF;
        } else {
            buffer[i] = eeprom_read_byte((const uint8_t *)(STORAGE_SCRIPT_START + offset + i));
        }
    }
}

uint16_t storage_get_script_length(void) {
    return cache.valid ? cache.length : 0;
}

uint16_t storage_get_initial_delay(void) {
    if (!cache.valid) {
        return 0;
    }
    return cache.delay * 100;
}

/* Writing */

void storage_write_byte(uint16_t offset, uint8_t value) {
    if (offset < STORAGE_MAX_SCRIPT_SIZE) {
        eeprom_update_byte((uint8_t *)(STORAGE_SCRIPT_START + offset), value);
    }
}

void storage_write_bytes(uint16_t offset, const uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        if (offset + i < STORAGE_MAX_SCRIPT_SIZE) {
            eeprom_update_byte((uint8_t *)(STORAGE_SCRIPT_START + offset + i), data[i]);
        }
    }
}

void storage_write_header(uint8_t version, uint8_t flags, uint16_t delay, uint16_t length, uint16_t crc) {
    eeprom_update_byte((uint8_t *)HEADER_OFFSET_VERSION, version);
    eeprom_update_byte((uint8_t *)HEADER_OFFSET_FLAGS, flags);
    write_u16(HEADER_OFFSET_DELAY, delay);
    write_u16(HEADER_OFFSET_LENGTH, length);
    write_u16(HEADER_OFFSET_CRC, crc);

    cache.flags = flags;
    cache.delay = delay;
    cache.length = length;
    cache.valid = (length > 0) && (length <= STORAGE_MAX_SCRIPT_SIZE);
}

void storage_invalidate_script(void) {
    write_u16(HEADER_OFFSET_LENGTH, 0);
    cache.valid = false;
    cache.length = 0;
}

/* Validation */

bool storage_has_valid_script(void) {
    return cache.valid;
}

bool storage_verify_crc(uint16_t length, uint16_t expected_crc) {
    if (length == 0 || length > STORAGE_MAX_SCRIPT_SIZE) {
        return false;
    }

    uint16_t crc = crc16_init();
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = eeprom_read_byte((const uint8_t *)(STORAGE_SCRIPT_START + i));
        crc = crc16_update(crc, byte);
    }
    crc = crc16_finalize(crc);

    return crc == expected_crc;
}

