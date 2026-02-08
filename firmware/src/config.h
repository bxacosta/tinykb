/**
 * config.h - Centralized firmware configuration constants
 *
 * All shared constants are defined here. Module-specific constants
 * remain in their respective headers.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* -------------------------------------------------------------------------- */
/* Hardware Configuration                                                     */
/* -------------------------------------------------------------------------- */

#define HW_EEPROM_SIZE            512   /* ATtiny85 EEPROM size in bytes */

/* -------------------------------------------------------------------------- */
/* Protocol Configuration                                                     */
/* -------------------------------------------------------------------------- */

#define PROTOCOL_REPORT_SIZE      32    /* HID report size in bytes    */
#define PROTOCOL_FIRMWARE_VERSION 0x01  /* Firmware version for STATUS */

/* -------------------------------------------------------------------------- */
/* Storage Header Layout                                                      */
/* -------------------------------------------------------------------------- */

#define STORAGE_HEADER_SIZE       8     /* Script header size in bytes     */
#define STORAGE_MODE_FLAG_SIZE    1     /* Mode flag uses last EEPROM byte */

/* Header field offsets (all multi-byte fields are little-endian) */
#define HEADER_OFFSET_VERSION     0     /* 1 byte  */
#define HEADER_OFFSET_FLAGS       1     /* 1 byte  */
#define HEADER_OFFSET_DELAY       2     /* 2 bytes */
#define HEADER_OFFSET_LENGTH      4     /* 2 bytes */
#define HEADER_OFFSET_CRC         6     /* 2 bytes */

/* Header validation */
#define STORAGE_PAYLOAD_VERSION   0x1A  /* Payload format version */

/* -------------------------------------------------------------------------- */
/* Storage Layout (Derived)                                                   */
/* -------------------------------------------------------------------------- */

#define STORAGE_EEPROM_SIZE       HW_EEPROM_SIZE
#define STORAGE_SCRIPT_START      STORAGE_HEADER_SIZE
#define STORAGE_MAX_SCRIPT_SIZE   (STORAGE_EEPROM_SIZE - STORAGE_HEADER_SIZE - STORAGE_MODE_FLAG_SIZE)
#define STORAGE_MODE_FLAG_ADDR    (STORAGE_EEPROM_SIZE - STORAGE_MODE_FLAG_SIZE)

/* -------------------------------------------------------------------------- */
/* Protocol Data Limits (Derived)                                             */
/* -------------------------------------------------------------------------- */

/* Control bytes overhead per command */
#define PROTOCOL_WRITE_OVERHEAD   5     /* cmd(1) + addr(2) + len(2) */
#define PROTOCOL_READ_OVERHEAD    3     /* status(1) + bytes_read(2) */
#define PROTOCOL_APPEND_OVERHEAD  3     /* cmd(1) + len(2)           */

/* Maximum data payload per command (report size - overhead) */
#define PROTOCOL_MAX_WRITE_DATA   (PROTOCOL_REPORT_SIZE - PROTOCOL_WRITE_OVERHEAD)
#define PROTOCOL_MAX_READ_DATA    (PROTOCOL_REPORT_SIZE - PROTOCOL_READ_OVERHEAD)
#define PROTOCOL_MAX_APPEND_DATA  (PROTOCOL_REPORT_SIZE - PROTOCOL_APPEND_OVERHEAD)

/* -------------------------------------------------------------------------- */
/* CRC Configuration                                                          */
/* -------------------------------------------------------------------------- */

#define CRC16_INIT                0xFFFF
#define CRC16_POLY                0x1021

#endif /* CONFIG_H */
