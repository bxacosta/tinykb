/**
 * hid_protocol.c - HID report protocol for programming mode
 *
 * Implements the command protocol (WRITE, READ, APPEND, COMMIT, etc.)
 * for programming scripts via WebHID.
 *
 * Protocol details: firmware/spec/hid-report-protocol.md
 */

#include "hid_protocol.h"
#include "eeprom_storage.h"
#include "crc16.h"
#include <string.h>

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static uint8_t response[PROTOCOL_REPORT_SIZE];
static uint8_t response_length;
static uint16_t current_offset;
static uint16_t running_crc;
static bool exit_requested;

/* Helpers */

static void set_ok_response(void) {
    response[0] = PROTOCOL_STATUS_OK;
    response_length = 1;
}

static void set_error_response(uint8_t status) {
    response[0] = status;
    response_length = 1;
}

static uint16_t read_le16(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static void write_le16(uint8_t *data, uint16_t value) {
    data[0] = (uint8_t)(value & 0xFF);
    data[1] = (uint8_t)(value >> 8);
}

/* Command Handlers */

static void handle_write_command(const uint8_t *report) {
    uint16_t address = read_le16(&report[1]);
    uint16_t length = read_le16(&report[3]);

    /* Validate address (absolute EEPROM address) */
    if (address >= STORAGE_EEPROM_SIZE) {
        set_error_response(PROTOCOL_STATUS_INVALID_ADDRESS);
        return;
    }

    /* Validate length */
    if (length == 0 || length > PROTOCOL_MAX_WRITE_DATA ||
        (address + length) > STORAGE_EEPROM_SIZE) {
        set_error_response(PROTOCOL_STATUS_INVALID_LENGTH);
        return;
    }

    /* Write bytes to EEPROM (absolute address) */
    storage_write_bytes(address, &report[5], length);

    /* Response: status(1) + bytes_written(2) */
    response[0] = PROTOCOL_STATUS_OK;
    write_le16(&response[1], length);
    response_length = 3;
}

static void handle_read_command(const uint8_t *report) {
    uint16_t address = read_le16(&report[1]);
    uint16_t length = read_le16(&report[3]);

    /* Validate address (absolute EEPROM address) */
    if (address >= STORAGE_EEPROM_SIZE) {
        set_error_response(PROTOCOL_STATUS_INVALID_ADDRESS);
        return;
    }

    /* Validate length */
    if (length == 0 || length > PROTOCOL_MAX_READ_DATA ||
        (address + length) > STORAGE_EEPROM_SIZE) {
        set_error_response(PROTOCOL_STATUS_INVALID_LENGTH);
        return;
    }

    /* Response: status(1) + bytes_read(2) + data(N) */
    response[0] = PROTOCOL_STATUS_OK;
    write_le16(&response[1], length);
    
    /* Read bytes from EEPROM (absolute address) */
    storage_read_bytes(address, &response[3], length);

    response_length = 3 + (uint8_t)length;
}

static void handle_append_command(const uint8_t *report) {
    uint16_t length = read_le16(&report[1]);

    /* Validate length */
    if (length == 0 || length > PROTOCOL_MAX_APPEND_DATA ||
        (current_offset + length) > STORAGE_MAX_SCRIPT_SIZE) {
        set_error_response(PROTOCOL_STATUS_INVALID_LENGTH);
        return;
    }

    /* Write bytes and update CRC */
    for (uint16_t i = 0; i < length; i++) {
        uint8_t byte = report[3 + i];
        storage_write_byte(STORAGE_SCRIPT_START + current_offset + i, byte);
        running_crc = crc16_update(running_crc, byte);
    }

    current_offset += length;

    /* Response: status(1) + next_offset(2) + running_crc(2) */
    response[0] = PROTOCOL_STATUS_OK;
    write_le16(&response[1], current_offset);
    write_le16(&response[3], running_crc);
    response_length = 5;
}

static void handle_reset_command(void) {
    current_offset = 0;
    running_crc = crc16_init();

    set_ok_response();
}

static void handle_commit_command(const uint8_t *report) {
    uint8_t options = report[1];
    uint8_t version = report[2];
    uint8_t flags = report[3];
    uint16_t delay = read_le16(&report[4]);
    uint16_t length = read_le16(&report[6]);
    uint16_t expected_crc = read_le16(&report[8]);

    /* Validate length */
    if (length == 0 || length > STORAGE_MAX_SCRIPT_SIZE) {
        set_error_response(PROTOCOL_STATUS_INVALID_LENGTH);
        return;
    }

    /* Calculate CRC based on options */
    uint16_t calculated_crc;
    if (options & PROTOCOL_OPT_CRC_FROM_EEPROM) {
        /* Read EEPROM and calculate CRC */
        calculated_crc = crc16_init();
        for (uint16_t i = 0; i < length; i++) {
            uint8_t byte = storage_read_byte(STORAGE_SCRIPT_START + i);
            calculated_crc = crc16_update(calculated_crc, byte);
        }
        calculated_crc = crc16_finalize(calculated_crc);
    } else {
        /* Use running CRC */
        calculated_crc = crc16_finalize(running_crc);
    }

    /* Reset state regardless of result */
    current_offset = 0;
    running_crc = crc16_init();

    /* Validate CRC */
    if (calculated_crc != expected_crc) {
        /* Invalidate script */
        storage_invalidate_script();
        set_error_response(PROTOCOL_STATUS_CRC_MISMATCH);
        return;
    }

    /* Write header */
    storage_write_header(version, flags, delay, length, expected_crc);

    set_ok_response();
}

static void handle_status_command(void) {
    memset(response, 0, sizeof(response));

    response[0] = PROTOCOL_STATUS_OK;
    response[1] = PROTOCOL_FIRMWARE_VERSION;           /* FwVersion */
    write_le16(&response[2], STORAGE_EEPROM_SIZE);     /* EEPROMSize */
    response[4] = PROTOCOL_REPORT_SIZE;                /* ReportSize */
    write_le16(&response[5], running_crc);             /* RunningCRC */
    write_le16(&response[7], current_offset);          /* CurrentOffset */

    response_length = PROTOCOL_REPORT_SIZE;
}

static void handle_exit_command(void) {
    exit_requested = true;
    /* No response - device will reset */
    response_length = 0;
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void protocol_init(void) {
    current_offset = 0;
    running_crc = crc16_init();
    exit_requested = false;
    response_length = 0;
}

/* Command Processing */

void protocol_process_report(const uint8_t *report, uint8_t length) {
    if (length == 0) {
        set_error_response(PROTOCOL_STATUS_INVALID_COMMAND);
        return;
    }

    uint8_t command = report[0];

    switch (command) {
        case PROTOCOL_CMD_WRITE:
            handle_write_command(report);
            break;

        case PROTOCOL_CMD_READ:
            handle_read_command(report);
            break;

        case PROTOCOL_CMD_APPEND:
            handle_append_command(report);
            break;

        case PROTOCOL_CMD_RESET:
            handle_reset_command();
            break;

        case PROTOCOL_CMD_COMMIT:
            handle_commit_command(report);
            break;

        case PROTOCOL_CMD_STATUS:
            handle_status_command();
            break;

        case PROTOCOL_CMD_EXIT:
            handle_exit_command();
            break;

        default:
            set_error_response(PROTOCOL_STATUS_INVALID_COMMAND);
            break;
    }
}

/* Response Access */

const uint8_t* protocol_get_response(void) {
    return response;
}

uint8_t protocol_get_response_length(void) {
    return response_length;
}

bool protocol_exit_requested(void) {
    return exit_requested;
}
