/**
 * usb_vendor.c - USB vendor request handler
 *
 * Implements programming protocol for uploading scripts via USB.
 */

#include "usb_vendor.h"
#include "eeprom_storage.h"
#include "crc16.h"
#include <avr/eeprom.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define WRITE_BUFFER_SIZE   8   /* Max bytes per CMD_WRITE */
#define STATUS_BUFFER_SIZE  8   /* Size of status response */

/* Header offsets for direct EEPROM access */
#define OFFSET_MAGIC    0
#define OFFSET_VERSION  2
#define OFFSET_FLAGS    3
#define OFFSET_LENGTH   4
#define OFFSET_CRC      6

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* State */

static device_mode_t current_mode = MODE_INITIALIZING;

/* Write state (for multi-packet transfers) */
static uint16_t write_offset;       /* Current write position in script area */
static uint16_t write_remaining;    /* Bytes remaining in current write op */
static uint16_t running_crc;        /* CRC accumulated during writes */

/* Commit state */
static uint16_t commit_length;      /* Expected total length from host */
static uint16_t commit_crc;         /* Expected CRC from host */
static uint8_t  last_commit_status; /* Result of last commit */

/* Status buffer (sent in response to CMD_STATUS) */
static uint8_t status_buffer[STATUS_BUFFER_SIZE];
static uint8_t status_bytes_remaining;

/* Current request being processed */
static uint8_t current_command;

/* Helpers */

static void write_u16_eeprom(uint16_t offset, uint16_t value) {
    eeprom_write_byte((uint8_t *)offset, value & 0xFF);
    eeprom_write_byte((uint8_t *)(offset + 1), (value >> 8) & 0xFF);
}

static void prepare_status_buffer(void) {
    /* Byte 0: Device mode */
    status_buffer[0] = current_mode;

    /* Byte 1: Last commit status */
    status_buffer[1] = last_commit_status;

    /* Bytes 2-3: Current write offset (little-endian) */
    status_buffer[2] = write_offset & 0xFF;
    status_buffer[3] = (write_offset >> 8) & 0xFF;

    /* Bytes 4-5: Script length in EEPROM (little-endian) */
    uint16_t script_len = storage_get_script_length();
    status_buffer[4] = script_len & 0xFF;
    status_buffer[5] = (script_len >> 8) & 0xFF;

    /* Byte 6: Has valid script */
    status_buffer[6] = storage_has_valid_script() ? 1 : 0;

    /* Byte 7: Reserved */
    status_buffer[7] = 0;
}

/* -------------------------------------------------------------------------- */
/* Command Handlers                                                           */
/* -------------------------------------------------------------------------- */

static usbMsgLen_t handle_cmd_program(void) {
    /* Enter program mode - reset write state */
    current_mode = MODE_PROGRAM;
    write_offset = 0;
    running_crc = CRC16_INIT;
    last_commit_status = 0xFF;  /* Not committed yet */
    return 0;
}

static usbMsgLen_t handle_cmd_write(usbRequest_t *rq) {
    /* Only valid in program mode */
    if (current_mode != MODE_PROGRAM) {
        return 0;
    }

    /* wValue contains the offset for this chunk */
    uint16_t offset = rq->wValue.word;

    /* Validate offset matches expected position */
    if (offset != write_offset) {
        return 0;  /* Out of sequence */
    }

    /* wLength is how many bytes follow */
    write_remaining = rq->wLength.word;
    if (write_remaining > WRITE_BUFFER_SIZE) {
        write_remaining = WRITE_BUFFER_SIZE;
    }

    return USB_NO_MSG;  /* Request data via usbFunctionWrite */
}

static usbMsgLen_t handle_cmd_commit(usbRequest_t *rq) {
    /* wValue = total length, wIndex = expected CRC */
    commit_length = rq->wValue.word;
    commit_crc = rq->wIndex.word;

    /* Validate length */
    if (commit_length == 0 || commit_length > STORAGE_MAX_SCRIPT) {
        last_commit_status = COMMIT_ERR_SIZE_INVALID;
        goto send_status;
    }

    /* Validate length matches what we received */
    if (commit_length != write_offset) {
        last_commit_status = COMMIT_ERR_SIZE_INVALID;
        goto send_status;
    }

    /* Validate CRC */
    if (commit_crc != running_crc) {
        last_commit_status = COMMIT_ERR_CRC_MISMATCH;
        goto send_status;
    }

    /* Write header to EEPROM */
    write_u16_eeprom(OFFSET_MAGIC, STORAGE_MAGIC);
    eeprom_write_byte((uint8_t *)OFFSET_VERSION, STORAGE_VERSION);
    eeprom_write_byte((uint8_t *)OFFSET_FLAGS, 0x02);  /* 1s initial delay */
    write_u16_eeprom(OFFSET_LENGTH, commit_length);
    write_u16_eeprom(OFFSET_CRC, commit_crc);

    /* Reinitialize storage to pick up new script */
    storage_init();

    /* Verify it worked */
    if (!storage_has_valid_script()) {
        last_commit_status = COMMIT_ERR_WRITE_FAILED;
        goto send_status;
    }

    last_commit_status = COMMIT_SUCCESS;

send_status:
    /* Return single byte status via usbFunctionRead */
    status_buffer[0] = last_commit_status;
    status_bytes_remaining = 1;
    usbMsgPtr = (usbMsgPtr_t)status_buffer;
    return USB_NO_MSG;  /* Use usbFunctionRead */
}

static usbMsgLen_t handle_cmd_execute(void) {
    /* Switch to running mode - main loop will start engine */
    current_mode = MODE_RUNNING;
    return 0;
}

static usbMsgLen_t handle_cmd_status(void) {
    /* Prepare and return status buffer */
    prepare_status_buffer();
    status_bytes_remaining = STATUS_BUFFER_SIZE;
    usbMsgPtr = (usbMsgPtr_t)status_buffer;
    return USB_NO_MSG;  /* Use usbFunctionRead */
}

static usbMsgLen_t handle_cmd_clear(void) {
    storage_clear();
    write_offset = 0;
    running_crc = CRC16_INIT;
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void vendor_init(void) {
    current_mode = MODE_INITIALIZING;
    write_offset = 0;
    running_crc = CRC16_INIT;
    last_commit_status = 0xFF;
}

/* Mode management */

device_mode_t vendor_get_mode(void) {
    return current_mode;
}

void vendor_set_mode(device_mode_t mode) {
    current_mode = mode;
}

bool vendor_is_program_mode(void) {
    return current_mode == MODE_PROGRAM;
}

/* -------------------------------------------------------------------------- */
/* USB Handlers (called by usb_core.c)                                        */
/* -------------------------------------------------------------------------- */

usbMsgLen_t vendor_handle_setup(usbRequest_t *rq) {
    current_command = rq->bRequest;

    switch (rq->bRequest) {
        case CMD_PROGRAM:
            return handle_cmd_program();

        case CMD_WRITE:
            return handle_cmd_write(rq);

        case CMD_COMMIT:
            return handle_cmd_commit(rq);

        case CMD_EXECUTE:
            return handle_cmd_execute();

        case CMD_STATUS:
            return handle_cmd_status();

        case CMD_CLEAR:
            return handle_cmd_clear();

        default:
            return 0;
    }
}

usbMsgLen_t vendor_handle_write(uint8_t *data, uint8_t len) {
    /* Called for CMD_WRITE data packets */
    if (current_command != CMD_WRITE || current_mode != MODE_PROGRAM) {
        return 1;  /* Finish transfer */
    }

    /* Write bytes to EEPROM and update CRC */
    for (uint8_t i = 0; i < len && write_remaining > 0; i++) {
        eeprom_write_byte((uint8_t *)(STORAGE_HEADER_SIZE + write_offset), data[i]);
        running_crc = crc16_update(running_crc, data[i]);
        write_offset++;
        write_remaining--;
    }

    return (write_remaining == 0) ? 1 : 0;  /* 1 = done, 0 = more expected */
}

uint8_t vendor_handle_read(uint8_t *data, uint8_t len) {
    /* Called for CMD_STATUS and CMD_COMMIT responses */
    uint8_t to_send = len;
    if (to_send > status_bytes_remaining) {
        to_send = status_bytes_remaining;
    }

    for (uint8_t i = 0; i < to_send; i++) {
        data[i] = status_buffer[STATUS_BUFFER_SIZE - status_bytes_remaining + i];
    }

    status_bytes_remaining -= to_send;
    return to_send;
}
