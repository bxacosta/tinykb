# HID Programming Specification

Defines the HID report protocol for transferring, validating, and storing data on the device. Platform-agnostic and
compatible with any HID library.

---

## Design Goals

1. **Robust error detection**: Detect transmission and synchronization errors immediately, enabling instant recovery
   without waiting for transfer completion.
2. **Storage-safe writes**: Ensure data validity before persisting and avoid redundant writes to maximize transfer
   efficiency.
3. **Fixed report size**: Use constant-size reports to simplify buffer allocation and eliminate dynamic memory
   requirements on resource-constrained devices.

---

## Report Format

All communication uses 32-byte HID reports with no Report ID. Multi-byte integers use **Little-Endian** byte order (LSB
first).

### Output Report (Host → Device)

| Offset | Field   | Size | Description                 |
|--------|---------|------|-----------------------------|
| 0      | Command | 1    | Command opcode (0x01-0x07)  |
| 1-31   | Payload | 31   | Command-specific parameters |

### Input Report (Device → Host)

| Offset | Field   | Size | Description                    |
|--------|---------|------|--------------------------------|
| 0      | Status  | 1    | Result code (see Status Codes) |
| 1-31   | Payload | 31   | Command-specific response      |

---

## Command Set

| Code | Name   | Format                      | Type      | Description                       |
|------|--------|-----------------------------|-----------|-----------------------------------|
| 0x01 | WRITE  | WRITE(offset, length, data) | Stateless | Write bytes to storage area       |
| 0x02 | READ   | READ(offset, length)        | Stateless | Read bytes from storage area      |
| 0x03 | APPEND | APPEND(length, data)        | Stateful  | Sequential write with running CRC |
| 0x04 | RESET  | RESET()                     | Stateful  | Reset programming state           |
| 0x05 | COMMIT | COMMIT(options, header)     | Stateful  | Validate CRC and write header     |
| 0x06 | STATUS | STATUS()                    | Stateless | Get device state and capabilities |
| 0x07 | EXIT   | EXIT()                      | -         | Exit programming mode             |

---

## State Variables

| Variable       | Initial | Modified by           | Description                              |
|----------------|---------|-----------------------|------------------------------------------|
| Current offset | 0       | APPEND, RESET, COMMIT | Next write position in storage area      |
| Running CRC    | 0xFFFF  | APPEND, RESET, COMMIT | Accumulated CRC-16-CCITT of written data |

---

## Status Codes

| Code | Name            | Description                   |
|------|-----------------|-------------------------------|
| 0x00 | OK              | Operation successful          |
| 0x01 | INVALID_COMMAND | Unknown command               |
| 0x02 | INVALID_ADDRESS | Offset out of range           |
| 0x03 | INVALID_LENGTH  | Length is 0 or exceeds limits |
| 0x04 | CRC_MISMATCH    | CRC validation failed         |

---

## Command Specifications

### WRITE (0x01)

Writes bytes to storage area at specified offset. Does not update running CRC.

**Format:** `WRITE(offset: uint16_le, length: uint16_le, data: bytes)`

**Request:**

| Offset | Field   | Size | Type     | Description         |
|--------|---------|------|----------|---------------------|
| 0      | COMMAND | 1    | uint8    | WRITE (0x01)        |
| 1-2    | OFFSET  | 2    | uint16_t | Storage offset (LE) |
| 3-4    | LENGTH  | 2    | uint16_t | Bytes to write (LE) |
| 5-31   | DATA    | 27   | bytes    | Data to write       |

**Response:**

| Offset | Field         | Size | Type     | Description               |
|--------|---------------|------|----------|---------------------------|
| 0      | STATUS        | 1    | uint8    | Result code               |
| 1-2    | BYTES_WRITTEN | 2    | uint16_t | Actual bytes written (LE) |

**Constraints:**

- Data length limited by report payload capacity
- Offset must not exceed storage boundaries

**Status:**

- `OK`: Data written successfully
- `INVALID_ADDRESS`: Offset exceeds storage boundaries
- `INVALID_LENGTH`: Length validation failed:
    - Cannot be zero
    - Cannot exceed per-report limit
    - Total write (offset + length) cannot exceed storage boundaries

**Examples:**

- Write 0xAB at offset 0: `WRITE(offset: 0, length: 1, data: [0xAB])` → `01 00 00 01 00 AB`
- Write 3 bytes at offset 10: `WRITE(offset: 10, length: 3, data: [0x01, 0x02, 0x03])` → `01 0A 00 03 00 01 02 03`

### READ (0x02)

Reads bytes from storage area at specified offset.

**Format:** `READ(offset: uint16_le, length: uint16_le)`

**Request:**

| Offset | Field   | Size | Type     | Description         |
|--------|---------|------|----------|---------------------|
| 0      | COMMAND | 1    | uint8    | READ (0x02)         |
| 1-2    | OFFSET  | 2    | uint16_t | Storage offset (LE) |
| 3-4    | LENGTH  | 2    | uint16_t | Bytes to read (LE)  |

**Response:**

| Offset | Field      | Size | Type     | Description            |
|--------|------------|------|----------|------------------------|
| 0      | STATUS     | 1    | uint8    | Result code            |
| 1-2    | BYTES_READ | 2    | uint16_t | Actual bytes read (LE) |
| 3-31   | DATA       | 29   | bytes    | Read data              |

**Constraints:**

- Read length limited by response payload capacity
- Offset must not exceed storage boundaries

**Status:**

- `OK`: Data read successfully
- `INVALID_ADDRESS`: Offset exceeds storage boundaries
- `INVALID_LENGTH`: Length validation failed:
    - Cannot be zero
    - Cannot exceed per-report limit
    - Total read (offset + length) cannot exceed storage boundaries

**Examples:**

- Read 10 bytes from offset 0: `READ(offset: 0, length: 10)` → `02 00 00 0A 00`

### APPEND (0x03)

Writes data at current offset position while computing CRC incrementally. Enables chunked transfers.

**Format:** `APPEND(length: uint16_le, data: bytes)`

**Request:**

| Offset | Field   | Size | Type     | Description          |
|--------|---------|------|----------|----------------------|
| 0      | COMMAND | 1    | uint8    | APPEND (0x03)        |
| 1-2    | LENGTH  | 2    | uint16_t | Bytes to append (LE) |
| 3-31   | DATA    | 29   | bytes    | Data to append       |

**Response:**

| Offset | Field       | Size | Type     | Description              |
|--------|-------------|------|----------|--------------------------|
| 0      | STATUS      | 1    | uint8    | Result code              |
| 1-2    | NEXT_OFFSET | 2    | uint16_t | Next write position (LE) |
| 3-4    | RUNNING_CRC | 2    | uint16_t | Accumulated CRC16 (LE)   |

**Constraints:**

- Data length limited by report payload capacity
- Write offset resets to the initial value when RESET is called

**Behavior:**

1. Writes data at current offset position
2. Updates running CRC with appended bytes
3. Advances offset by data length

**Status:**

- `OK`: Data appended successfully
- `INVALID_LENGTH`: Length validation failed:
    - Cannot be zero
    - Cannot exceed per-report limit
    - Total write (current offset + length) cannot exceed storage boundaries

**Examples:**

- Append 5 bytes: `APPEND(length: 5, data: [0x08, 0x05, 0x48, 0x65, 0x6C])` → `03 05 00 08 05 48 65 6C`

### RESET (0x04)

Resets programming state to initial values. Does not modify persistent storage.

**Format:** `RESET()`

**Request:**

| Offset | Field   | Size | Type  | Description  |
|--------|---------|------|-------|--------------|
| 0      | COMMAND | 1    | uint8 | RESET (0x04) |

**Response:**

| Offset | Field  | Size | Type  | Description |
|--------|--------|------|-------|-------------|
| 0      | STATUS | 1    | uint8 | Result code |

**Behavior:**

- Resets write position to start of storage area
- Initializes CRC accumulator to initial value

**Status:**

- `OK`: State reset successfully

**Examples:**

- Reset state: `RESET()` → `04`

### COMMIT (0x05)

Validates data integrity and writes metadata header to storage.

**Format:**
`COMMIT(options: uint8, version: uint8, flags: uint8, delay: uint16_le, length: uint16_le, crc16: uint16_le)`

**Request:**

| Offset | Field   | Size | Type     | Description                       |
|--------|---------|------|----------|-----------------------------------|
| 0      | COMMAND | 1    | uint8    | COMMIT (0x05)                     |
| 1      | OPTIONS | 1    | uint8    | Command options                   |
| 2      | VERSION | 1    | uint8    | Payload format version identifier |
| 3      | FLAGS   | 1    | uint8    | Reserved flags                    |
| 4-5    | DELAY   | 2    | uint16_t | Pre-execution delay (LE)          |
| 6-7    | LENGTH  | 2    | uint16_t | Data length in bytes (LE)         |
| 8-9    | CRC16   | 2    | uint16_t | Expected CRC16 (LE)               |

**Response:**

| Offset | Field  | Size | Type  | Description |
|--------|--------|------|-------|-------------|
| 0      | STATUS | 1    | uint8 | Result code |

**Options Bitmap:**

- Bit 0 (CRC_MODE): CRC validation mode
    - `0`: Validation against running CRC accumulator
    - `1`: Recalculate CRC from stored data before validation
- Bits 1-7: Reserved (ignored)

**Behavior:**

1. Validates data length is within storage boundaries
2. Validates CRC integrity:
    - Options bit 0 = 0: Compares provided CRC with running CRC accumulator
    - Options bit 0 = 1: Recalculates CRC from stored data and compares with provided CRC
3. If valid: Writes header to storage, resets state
4. If invalid: Invalidates header (sets LENGTH=0), resets state

**Status:**

- `OK`: Header written and data validated successfully
- `INVALID_LENGTH`: Length validation failed:
    - Cannot be zero
    - Cannot exceed storage capacity
- `CRC_MISMATCH`: Computed CRC does not match expected CRC16

**Examples:**

- Commit with running CRC: `COMMIT(options: 0, version: 0x1A, flags: 0, delay: 0, length: 10, crc: 0xD186)` →
  `05 00 1A 00 00 00 0A 00 86 D1`
- Commit with recalculation: `COMMIT(options: 1, version: 0x1A, ...)` → `05 01 1A 00 ...`

### STATUS (0x06)

Returns current device state and capabilities.

**Format:** `STATUS()`

**Request:**

| Offset | Field   | Size | Type  | Description   |
|--------|---------|------|-------|---------------|
| 0      | COMMAND | 1    | uint8 | STATUS (0x06) |

**Response:**

| Offset | Field            | Size | Type     | Description               |
|--------|------------------|------|----------|---------------------------|
| 0      | STATUS           | 1    | uint8    | Result code               |
| 1      | FIRMWARE_VERSION | 1    | uint8    | Firmware version          |
| 2-3    | STORAGE_SIZE     | 2    | uint16_t | Total storage size (LE)   |
| 4      | REPORT_SIZE      | 1    | uint8    | HID report size           |
| 5-6    | RUNNING_CRC      | 2    | uint16_t | Current CRC state (LE)    |
| 7-8    | CURRENT_OFFSET   | 2    | uint16_t | Current write offset (LE) |

**Status:**

- `OK`: Status retrieved successfully

**Examples:**

- Get status: `STATUS()` → `06`

### EXIT (0x07)

Exits programming mode.

**Format:** `EXIT()`

**Request:**

| Offset | Field   | Size | Type  | Description |
|--------|---------|------|-------|-------------|
| 0      | COMMAND | 1    | uint8 | EXIT (0x07) |

**Response:**

- No response (device resets)

**Behavior:**

1. Persists mode transition flag
2. Triggers device reset

**Examples:**

- Exit programming mode: `EXIT()` → `07`

---

## Complete Examples

### Example 1

**Description:** Read status and header data

**Programming Sequence:**

| Step | Command                    | Request Bytes    | Response Description                                                                         | Response Bytes                     |
|------|----------------------------|------------------|----------------------------------------------------------------------------------------------|------------------------------------|
| 1    | STATUS()                   | `06`             | OK, FIRMWARE_VERSION=1, STORAGE_SIZE=512, REPORT_SIZE=32, CRC=0xFFFF, OFFSET=0               | `00 01 00 02 20 FF FF 00 00`       |
| 2    | READ(offset: 0, length: 8) | `02 00 00 08 00` | OK, BYTES_READ=8, DATA=[VERSION=0x1A, FLAGS=0x00, DELAY=0x0000, LENGTH=0x0000, CRC16=0xFFFF] | `00 08 00 1A 00 00 00 00 00 FF FF` |

### Example 2

**Description:** Upload script

**Script Bytecode:**

```
08 05 48 65 6C 6C 6F  ; STRING "Hello"
05 28                 ; TAP Enter
00                    ; END
```

**Header Bytecode:**

```
1A     ; VERSION 
00     ; FLAGS
14 00  ; DELAY 20*100ms
0A 00  ; LENGTH 10 bytes
86 D1  ; CRC16
```

**Programming Sequence:**

| Step | Command                              | Request Bytes                            | Response Description                   | Response Bytes   |
|------|--------------------------------------|------------------------------------------|----------------------------------------|------------------|
| 1    | RESET()                              | `04`                                     | OK                                     | `00`             |
| 2    | APPEND(length: 10, data: `[script]`) | `03 0A 00 08 05 48 65 6C 6C 6F 05 28 00` | OK, NEXT_OFFSET=10, RUNNING_CRC=0xD186 | `00 0A 00 86 D1` |
| 3    | COMMIT(opts: 0, `[header]`)          | `05 00 1A 00 14 00 0A 00 86 D1`          | OK                                     | `00`             |
| 4    | EXIT()                               | `07`                                     | -                                      | -                |

--- 

## Implementation Notes

Specific implementation values (storage capacity, payload limits, etc.) are hardware-dependent and should be defined
according to device constraints.

**Addressing:**

- Header is written only by COMMIT
- Mode flag is written only by EXIT

**CRC-16-CCITT Algorithm:**

- Polynomial: 0x1021
- Initial value: 0xFFFF
- Process: MSB first
- No final XOR

**USB Configuration:**

- Interface: HID (class 0x03)
- Subclass: 0x00 (no boot protocol)
- Protocol: 0x00
- Usage Page: 0xFF00 (Vendor Defined)
- Usage: 0x01

---

## Future Improvements

### New Commands

- **CLEAR**: Write header with default values.
-

---

## References

- [USB HID Specification](https://usb.org/sites/default/files/hid1_11.pdf)
- [WebHID API](https://developer.mozilla.org/en-US/docs/Web/API/WebHID_API)
- [V-USB Library](https://www.obdev.at/products/vusb/index.html)

---

## Changelog

### v1.0 (2025-02-08)

- Initial specification release
- Established 32-byte HID reports size
- Defined 7 commands: WRITE, READ, APPEND, RESET, COMMIT, STATUS, EXIT

