# Application Bytecode Specification

This document defines the binary format for the application script engine.

## 1. Storage Container Format

The payload stored in the EEPROM consists of an 8-byte header followed by the raw variable-length bytecode. All
multi-byte integer fields in the header are stored in **Little-Endian** byte order (LSB first), consistent with the AVR
architecture.

### Header Structure

The header occupies the first 8 bytes of the storage medium (Offset 0x00 - 0x07).

| Offset   | Field     | Size    | Type     | Value / Description                                |
|:---------|:----------|:--------|:---------|:---------------------------------------------------|
| **0x00** | `VERSION` | 1 byte  | uint8_t  | **0xA1**. Magic byte + format version v1 combined. |
| **0x01** | `FLAGS`   | 1 byte  | uint8_t  | Reserved for future use (set to 0x00).             |
| **0x02** | `DELAY`   | 2 bytes | uint16_t | Initial delay multiplier × 100ms [LE].             |
| **0x04** | `LENGTH`  | 2 bytes | uint16_t | Bytecode length in bytes (excluding header).       |
| **0x06** | `CRC16`   | 2 bytes | uint16_t | CRC-16-CCITT checksum of the bytecode payload.     |

### Initial Delay

The `DELAY` field at offset 0x02-0x03 specifies the wait time before script execution begins:

- Value is multiplied by 100ms
- Range: 0 to 65535 (0 to ~109 minutes)
- Example: `0x000A` (10) = 10 × 100ms = 1000ms (1 second)

### Script Validation

A script is considered valid when:

1. `VERSION == 0xA1`
2. `LENGTH > 0`

### Checksum Calculation

The `CRC16` field must be calculated over the entire bytecode payload (starting at Offset 0x08). The algorithm used is *
*CRC-16-CCITT** (Polynomial `0x1021`, Initial Value `0xFFFF`).

---

## 2. Bytecode Instruction Set

The bytecode follows the header starting at Offset 0x08. Instructions are variable-length. The first byte of each
instruction is the **Opcode**.

### Opcode Summary

| Opcode   | Mnemonic   | Parameters         | Format               | Description                                 |
|:---------|:-----------|:-------------------|:---------------------|:--------------------------------------------|
| **0x00** | `END`      | None               | `[END]`              | Terminates execution and releases all keys. |
| **0x01** | `DELAY`    | `[LO] [HI]`        | `[DELAY, LO, HI]`    | Pauses execution for N milliseconds.        |
| **0x02** | `KEY_DOWN` | `[KEYCODE]`        | `[KEY_DOWN, KEY]`    | Presses and holds a key.                    |
| **0x03** | `KEY_UP`   | `[KEYCODE]`        | `[KEY_UP, KEY]`      | Releases a key.                             |
| **0x04** | `MOD`      | `[MASK]`           | `[MOD, MASK]`        | Sets the active modifier keys.              |
| **0x05** | `TAP`      | `[KEYCODE]`        | `[TAP, KEY]`         | Atomically presses and releases a key.      |
| **0x06** | `REPEAT`   | `[CNT] [LEN] ...`  | `[REPEAT, N, LEN]`   | Repeats a block of code N times.            |
| **0x07** | `COMBO`    | `[MOD] [KEY]`      | `[COMBO, MOD, KEY]`  | Executes `TAP` with temporary modifiers.    |
| **0x08** | `STRING`   | `[LEN] [CHARS...]` | `[STRING, LEN, ...]` | Types a sequence of ASCII characters.       |

---

## 3. Instruction Reference

### 0x00: END

Terminates the script execution. The engine automatically releases all pressed keys and resets modifiers upon execution.
Every script **must** end with this opcode.

### 0x01: DELAY

Pauses execution for a specified duration while maintaining the current key state (pressed keys remain held).

- **Parameter 1**: `Duration Low Byte` (uint8)
- **Parameter 2**: `Duration High Byte` (uint8)
- **Format**: Little-Endian (LSB, MSB).
- **Range**: 0 to 65535 milliseconds (~65.53 seconds).

### 0x02: KEY_DOWN

Adds a key to the active report. The key remains pressed until a corresponding `KEY_UP` or `END` is executed.

- **Parameter 1**: `Keycode` (uint8) - USB HID Usage ID.
- **Constraint**: Maximum of 6 simultaneous keys (6KRO). Additional keys are ignored.

### 0x03: KEY_UP

Removes a key from the active report.

- **Parameter 1**: `Keycode` (uint8) - USB HID Usage ID.

### 0x04: MOD

Sets the absolute state of the modifier keys (Control, Shift, Alt, GUI). This replaces the current modifier state.

- **Parameter 1**: `Modifier Mask` (uint8)
    - Bit 0: Left Ctrl
    - Bit 1: Left Shift
    - Bit 2: Left Alt
    - Bit 3: Left GUI
    - Bit 4: Right Ctrl
    - Bit 5: Right Shift
    - Bit 6: Right Alt
    - Bit 7: Right GUI

### 0x05: TAP

Performs a `KEY_DOWN` followed immediately by a `KEY_UP` for the specified key.

- **Parameter 1**: `Keycode` (uint8)

### 0x06: REPEAT

Repeats a sequence of following bytes a specified number of times.

- **Parameter 1**: `Count` (uint8) - Number of iterations (1-255).
- **Parameter 2**: `Length` (uint8) - Size of the code block to repeat, in bytes.
- **Constraint**: Nested loops are not supported.

### 0x07: COMBO

Executes a keystroke with a specific modifier set, preserving the previous modifier state. Equivalent to: Save Mods ->
Set Mods -> Tap Key -> Restore Mods.

- **Parameter 1**: `Modifier Mask` (uint8)
- **Parameter 2**: `Keycode` (uint8)

### 0x08: STRING

types a sequence of text. The engine translates ASCII characters to keystrokes assuming a US Keyboard Layout.

- **Parameter 1**: `Length` (uint8) - Number of characters.
- **Parameter 2...N**: `Characters` (ASCII bytes).
- **Supported Range**: ASCII 0x08 (Backspace), 0x09 (Tab), 0x0A (Newline), and 0x20-0x7E (Printable).
- **Note**: Characters requiring Shift (e.g., 'A', '!', ':') are handled automatically.

---

## 4. Examples

The following examples demonstrate how to construct valid bytecode scripts for common tasks.

### Type "Hello" and Press Enter

Types the string "Hello" and immediately presses the Enter key.

- **Length**: 10 bytes (0x000A)
- **CRC16**: 0xD186

```text
Header:
A1          ; [VERSION=A1] (Magic + Version v1)
00          ; [FLAGS=00]
00 00       ; [DELAY=0000] (0 ms)
0A 00       ; [LENGTH=000A] (10 bytes)
86 D1       ; [CRC16=D186]

Bytecode (Offset 0x08):
08 05 48 65 6C 6C 6F ; [STRING=08, Length=05, H=48, e=65, l=6C, l=6C, o=6F]
05 28                ; [TAP=05, Enter=28]
00                   ; [END=00]

Full Payload: A1 00 00 00 0A 00 86 D1 08 05 48 65 6C 6C 6F 05 28 00
```

### Notepad Copy/Paste Workflow

Advanced macro that performs `Ctrl+A` (Select All), `Ctrl+C` (Copy), opens Notepad via Run dialog, and `Ctrl+V` (Pastes)
the content.

- **Length**: 32 bytes (0x0020)
- **CRC16**: 0xCB27

```text
Header:
A1          ; [VERSION=A1] (Magic + Version v1)
00          ; [FLAGS=00]
00 00       ; [DELAY=0000] (0 ms)
20 00       ; [LENGTH=0020] (32 bytes)
27 CB       ; [CRC16=CB27]

Bytecode (Offset 0x08):
07 01 04                   ; [COMBO=07, Mods=01, A=04] (Select All)
07 01 06                   ; [COMBO=07, Mods=01, C=06] (Copy)
07 08 15                   ; [COMBO=07, Mods=08, R=15] (Win+R)
01 F4 01                   ; [DELAY=01, Low=F4, High=01] (500 ms)
08 07 6E 6F 74 65 70 61 64 ; [STRING=08, Len=07, notepad]
05 28                      ; [TAP=05, Enter=28]
01 F4 01                   ; [DELAY=01, Low=F4, High=01] (500 ms)
07 01 19                   ; [COMBO=07, Mods=01, V=19] (Paste)
05 28                      ; [TAP=05, Enter=28]
00                         ; [END=00]

Full Payload: A1 00 00 00 20 00 27 CB 07 01 04 07 01 06 07 08 15 01 F4 01 08 07 6E 6F 74 65 70 61 64 05 28 01 F4 01 07 01 19 05 28 00
```

### Calculator Sum (Repeat Loop)

Opens the Calculator and computes "2222 + 777" using `REPEAT` loops to type the digits efficiently.

- **Length**: 33 bytes (0x0021)
- **CRC16**: 0x6FB4

```text
Header:
A1          ; [VERSION=A1] (Magic + Version v1)
00          ; [FLAGS=00]
00 00       ; [DELAY=0000] (0 ms)
21 00       ; [LENGTH=0021] (33 bytes)
B4 6F       ; [CRC16=6FB4]

Bytecode (Offset 0x08):
07 08 15             ; [COMBO=07, Mods=08, R=15] (Win+R)
01 F4 01             ; [DELAY=01, Low=F4, High=01] (500 ms)
08 04 63 61 6C 63    ; [STRING=08, Len=04, calc]
05 28                ; [TAP=05, Enter=28]
01 E8 03             ; [DELAY=01, Low=E8, High=03] (1000 ms)
06 04 02             ; [REPEAT=06, Count=04, Len=02] (Type '2' 4 times)
05 1F                ; [TAP=05, 2=1F]
08 01 2B             ; [STRING=08, Len=01, +=2B]
06 03 02             ; [REPEAT=06, Count=03, Len=02] (Type '7' 3 times)
05 24                ; [TAP=05, 7=24]
05 28                ; [TAP=05, Enter=28]
00                   ; [END=00]

Full Payload: A1 00 00 00 21 00 B4 6F 07 08 15 01 F4 01 08 04 63 61 6C 63 05 28 01 E8 03 06 04 02 05 1F 08 01 2B 06 03 02 05 24 05 28 00
```