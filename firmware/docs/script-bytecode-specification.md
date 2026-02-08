# Script Bytecode Specification

This document specifies the binary instruction format used by the keyboard script engine to execute automated
keystrokes.

---

## Design Goals

1. **Full keyboard capability**: Enable any behavior a physical keyboard can perform, leaving no use case impossible to
   express in bytecode.
2. **Minimal bytecode size**: Reduce script size for common operations through purpose-built shortcuts, optimizing
   storage and transmission efficiency.
3. **Guaranteed clean state**: Prevent stuck keys or orphaned modifier states under all circumstances, including script
   errors or unexpected termination.

## Storage Format

The script payload consists of an 8-byte header followed by variable-length bytecode. All multi-byte integers use
**Little-Endian** byte order (LSB first).

### Header Layout (Offsets 0x00-0x07)

| Offset | Field   | Size | Type     | Default | Description                                    |
|--------|---------|------|----------|---------|------------------------------------------------|
| 0x00   | VERSION | 1    | uint8_t  | 0x1A    | Payload format version identifier              |
| 0x01   | FLAGS   | 1    | uint8_t  | 0x00    | Reserved. Set to 0x00                          |
| 0x02   | DELAY   | 2    | uint16_t | 0x0000  | Pre-execution delay in 100ms units (LE)        |
| 0x04   | LENGTH  | 2    | uint16_t | 0x0000  | Bytecode size in bytes, excluding header (LE)  |
| 0x06   | CRC16   | 2    | uint16_t | 0xFFFF  | CRC-16-CCITT checksum of bytecode payload (LE) |

The `VERSION` field defines the payload format version. Must be set to `0x1A` for the current specification. Future
breaking changes will update this identifier to prevent execution on unsupported interpreters.

**Pre-execution Delay:**

- Calculated as: `DELAY × 100ms`
- Range: 0 to ~6553 seconds (~109 minutes)
- Example: 0x000A = 10 × 100ms = 1 second

**Validation Requirements:**

- VERSION must match the expected payload format version
- LENGTH must be greater than 0
- CRC16 must match computed checksum of bytecode

**Bytecode Location:** Bytecode starts immediately after header at offset 0x08.

---

## Instruction Set

All instructions begin with a single-byte opcode followed by zero or more parameter bytes. Instructions are executed
sequentially until END is reached.

### Opcode Reference

| Opcode | Name     | Format                                  | Description                                                    |
|--------|----------|-----------------------------------------|----------------------------------------------------------------|
| 0x00   | END      | END()                                   | Terminate execution, release all keys                          |
| 0x01   | DELAY    | DELAY(duration: uint16_le)              | Pauses execution for `duration` milliseconds                   |
| 0x02   | KEY_DOWN | KEY_DOWN(keycode: uint8)                | Presses and holds `keycode` until released                     |
| 0x03   | KEY_UP   | KEY_UP(keycode: uint8)                  | Releases the specified `keycode`                               |
| 0x04   | MOD      | MOD(mask: uint8)                        | Sets modifier state to `mask` (absolute replacement)           |
| 0x05   | TAP      | TAP(keycode: uint8)                     | Presses and immediately releases `keycode`                     |
| 0x06   | REPEAT   | REPEAT(iterations: uint8, size: uint8)  | Repeats the next `size` bytes `iterations` times               |
| 0x07   | COMBO    | COMBO(modifiers: uint8, keycode: uint8) | Taps `keycode` with temporary `modifiers`, then restores state |
| 0x08   | STRING   | STRING(text: string)                    | Types the ASCII `text` using US keyboard layout                |

---

## Instruction Specifications

### END (0x00)

Terminates script execution. Automatically releases all held keys and clears modifier state.

**Format:** `END()`

**Bytecode:** `0x00`

**Constraints:**

- Must be the final instruction in every valid script

**Examples:**

- Terminate script: `END()` → `0x00`

### DELAY (0x01)

Suspends execution while maintaining current keyboard state. Pressed keys remain held during delay.

**Format:** `DELAY(duration: uint16_le)`

**Bytecode:** `0x01 [duration_lo] [duration_hi]`

**Parameters:**

- duration: Delay in milliseconds (0-65535), encoded as little-endian

**Constraints:**

- Maximum delay: 65535ms (~65.5 seconds)

**Examples:**

- 100ms delay: `DELAY(duration: 100)` → `0x01 0x64 0x00`
- 1 second delay: `DELAY(duration: 1000)` → `0x01 0xE8 0x03`

### KEY_DOWN (0x02)

Presses and holds a key. The key remains in the active HID report until released by KEY_UP or END.

**Format:** `KEY_DOWN(keycode: uint8)`

**Bytecode:** `0x02 [keycode]`

**Parameters:**

- keycode: USB HID Usage ID of the key to press

**Constraints:**

- Maximum 6 simultaneous keys (6-key rollover limitation)
- Exceeding limit causes additional keys to be silently dropped
- Does not affect modifier keys (see MOD)

**Examples:**

- Hold 'A' key: `KEY_DOWN(keycode: KEY_A)` → `0x02 0x04`
- Hold Spacebar: `KEY_DOWN(keycode: KEY_SPACE)` → `0x02 0x2C`

### KEY_UP (0x03)

Releases a previously pressed key from the active HID report.

**Format:** `KEY_UP(keycode: uint8)`

**Bytecode:** `0x03 [keycode]`

**Parameters:**

- keycode: USB HID Usage ID of the key to release

**Examples:**

- Release 'A' key: `KEY_UP(keycode: KEY_A)` → `0x03 0x04`
- Release Spacebar: `KEY_UP(keycode: KEY_SPACE)` → `0x03 0x2C`

### MOD (0x04)

Sets modifier key state using absolute bitmask. Replaces current modifier state entirely.

**Format:** `MOD(mask: uint8)`

**Bytecode:** `0x04 [mask]`

**Parameters:**

- mask: Modifier bitmask where each bit represents a modifier key

**Modifier Bitmask:**

| Bit | Modifier    |
|-----|-------------|
| 0   | Left Ctrl   |
| 1   | Left Shift  |
| 2   | Left Alt    |
| 3   | Left GUI    |
| 4   | Right Ctrl  |
| 5   | Right Shift |
| 6   | Right Alt   |
| 7   | Right GUI   |

**Examples:**

- Clear all modifiers: `MOD(mask: 0x00)` → `0x04 0x00`
- Left Ctrl + Left Shift: `MOD(mask: 0x03)` → `0x04 0x03`

### TAP (0x05)

Presses and immediately releases a key. Equivalent to KEY_DOWN followed by KEY_UP.

**Format:** `TAP(keycode: uint8)`

**Bytecode:** `0x05 [keycode]`

**Parameters:**

- keycode: USB HID Usage ID of the key to tap

**Examples:**

- Press Enter: `TAP(keycode: KEY_ENTER)` → `0x05 0x28`
- Press Space: `TAP(keycode: KEY_SPACE)` → `0x05 0x2C`

### REPEAT (0x06)

Executes the next `size` bytes `iterations` times. Reduces script size for repetitive operations.

**Format:** `REPEAT(iterations: uint8, size: uint8)`

**Bytecode:** `0x06 [iterations] [size] [block...]`

**Parameters:**

- iterations: Number of times to repeat (1-255)
- size: Number of bytes in the block to repeat

**Constraints:**

- Nested REPEAT instructions are not supported
- Block must not contain partial instructions
- Maximum block size: 255 bytes

**Examples:**

- Type '2' four times: `REPEAT(iterations: 4, size: 2)` followed by `TAP(keycode: KEY_2)` → `0x06 0x04 0x02 0x05 0x1F`
- Type 'A' ten times: `REPEAT(iterations: 10, size: 2)` followed by `TAP(keycode: KEY_A)` → `0x06 0x0A 0x02 0x05 0x04`

### COMBO (0x07)

Taps a key with temporary modifiers applied. Previous modifier state is preserved and restored after execution.

**Format:** `COMBO(modifiers: uint8, keycode: uint8)`

**Bytecode:** `0x07 [modifiers] [keycode]`

**Parameters:**

- modifiers: Temporary modifier mask (same format as MOD)
- keycode: USB HID Usage ID of the key to tap with modifiers

**Examples:**

- Ctrl+C (Copy): `COMBO(modifiers: MOD_LCTRL, keycode: KEY_C)` → `0x07 0x01 0x06`
- Win+R (Run): `COMBO(modifiers: MOD_LGUI, keycode: KEY_R)` → `0x07 0x08 0x15`

### STRING (0x08)

Types ASCII text using US keyboard layout. Automatically handles shift state for uppercase letters and symbols.

**Format:** `STRING(text: string)`

**Bytecode:** `0x08 [length] [char1] [char2] ... [charN]`

**Parameters:**

- text: ASCII string to type (1-255 characters)

**Constraints:**

- Supported ASCII Printable Characters:
    - 0x20: Space
    - 0x21-0x2F: Symbols (`!`, `"`, `#`, `$`, `%`, `&`, `'`, `(`, `)`, `*`, `+`, `,`, `-`, `.`, `/`)
    - 0x30-0x39: Digits (0-9)
    - 0x3A-0x40: Symbols (`:`, `;`, `<`, `=`, `>`, `?`, `@`)
    - 0x41-0x5A: Uppercase letters (A-Z)
    - 0x5B-0x60: Symbols (`[`, `\`, `]`, `^`, `_`, `` ` ``)
    - 0x61-0x7A: Lowercase letters (a-z)
    - 0x7B-0x7E: Symbols (`{`, `|`, `}`, `~`)
- Unsupported characters are silently skipped
- Automatic shift handling for uppercase and symbols

**Examples:**

- Type "Hello": `STRING(text: "Hello")` → `0x08 0x05 0x48 0x65 0x6C 0x6C 0x6F`
- Type "Test!": `STRING(text: "Test!")` → `0x08 0x05 0x54 0x65 0x73 0x74 0x21`

---

## Complete Examples

### Example 1

**Description:** Type "Hello" and press `Enter`

**Script Bytecode:**

| Instruction | Hex Bytes            | Format                  | Details        |
|-------------|----------------------|-------------------------|----------------|
| STRING      | 08 05 48 65 6C 6C 6F | STRING(text: "Hello")   | length=0x05    |
| TAP         | 05 28                | TAP(keycode: KEY_ENTER) | KEY_ENTER=0x28 |
| END         | 00                   | END()                   | -              |

**Header Bytecode:**

| Offset | Field   | Value  | Description                       |
|--------|---------|--------|-----------------------------------|
| 0x00   | VERSION | 0x1A   | Script version v1                 |
| 0x01   | FLAGS   | 0x00   | Reserved                          |
| 0x02   | DELAY   | 0x0014 | Pre-execution delay 2s (20*100ms) |
| 0x04   | LENGTH  | 0x000A | Bytecode length (10 bytes)        |
| 0x06   | CRC16   | 0xD186 | Checksum                          |

**Complete Payload:**

```
Header: 1A 00 14 00 0A 00 86 D1
Payload: 08 05 48 65 6C 6C 6F 05 28 00
```

### Example 2

**Description:** Select all, copy, open Notepad and paste the content

**Script Bytecode:**

| Instruction | Hex Bytes                  | Format                                      | Details                    |
|-------------|----------------------------|---------------------------------------------|----------------------------|
| COMBO       | 07 01 04                   | COMBO(modifiers: MOD_LCTRL, keycode: KEY_A) | MOD_LCTRL=0x01, KEY_A=0x04 |
| COMBO       | 07 01 06                   | COMBO(modifiers: MOD_LCTRL, keycode: KEY_C) | MOD_LCTRL=0x01, KEY_C=0x06 |
| COMBO       | 07 08 15                   | COMBO(modifiers: MOD_LGUI, keycode: KEY_R)  | MOD_LGUI=0x08, KEY_R=0x15  |
| DELAY       | 01 F4 01                   | DELAY(duration: 500)                        | duration=0x01F4            |
| STRING      | 08 07 6E 6F 74 65 70 61 64 | STRING(text: "notepad")                     | length=0x07                |
| TAP         | 05 28                      | TAP(keycode: KEY_ENTER)                     | KEY_ENTER=0x28             |
| DELAY       | 01 F4 01                   | DELAY(duration: 500)                        | duration=0x01F4            |
| COMBO       | 07 01 19                   | COMBO(modifiers: MOD_LCTRL, keycode: KEY_V) | MOD_LCTRL=0x01, KEY_V=0x19 |
| END         | 00                         | END()                                       | -                          |

**Header Bytecode:**

| Offset | Field   | Value  | Description                |
|--------|---------|--------|----------------------------|
| 0x00   | VERSION | 0x1A   | Script version v1          |
| 0x01   | FLAGS   | 0x00   | Reserved                   |
| 0x02   | DELAY   | 0x0000 | Pre-execution delay (0ms)  |
| 0x04   | LENGTH  | 0x001E | Bytecode length (30 bytes) |
| 0x06   | CRC16   | 0xB746 | Checksum                   |

**Complete Payload:**

```
Header: 1A 00 00 00 1E 00 46 B7
Payload: 07 01 04 07 01 06 07 08 15 01 F4 01 08 07 6E 6F 74 65 70 61 64 05 28 01 F4 01 07 01 19 00
```

### Example 3

**Description:** Open calculator, compute 2222 + 777

**Script Bytecode:**

| Instruction | Hex Bytes         | Format                                     | Details                         |
|-------------|-------------------|--------------------------------------------|---------------------------------|
| COMBO       | 07 08 15          | COMBO(modifiers: MOD_LGUI, keycode: KEY_R) | MOD_LGUI=0x08, KEY_R=0x15       |
| DELAY       | 01 F4 01          | DELAY(duration: 500)                       | duration=0x01F4                 |
| STRING      | 08 04 63 61 6C 63 | STRING(text: "calc")                       | length=0x04, "calc"=63 61 6C 63 |
| TAP         | 05 28             | TAP(keycode: KEY_ENTER)                    | KEY_ENTER=0x28                  |
| DELAY       | 01 E8 03          | DELAY(duration: 1000)                      | duration=0x03E8                 |
| REPEAT      | 06 04 02          | REPEAT(iterations: 4, size: 2)             | iterations=0x04, size=0x02      |
| TAP         | 05 1F             | TAP(keycode: KEY_2)                        | KEY_2=0x1F                      |
| STRING      | 08 01 2B          | STRING(text: "+")                          | length=0x01, "+"=2B             |
| REPEAT      | 06 03 02          | REPEAT(iterations: 3, size: 2)             | iterations=0x03, size=0x02      |
| TAP         | 05 24             | TAP(keycode: KEY_7)                        | KEY_7=0x24                      |
| TAP         | 05 28             | TAP(keycode: KEY_ENTER)                    | KEY_ENTER=0x28                  |
| END         | 00                | END()                                      | -                               |

**Header Bytecode:**

| Offset | Field   | Value  | Description                |
|--------|---------|--------|----------------------------|
| 0x00   | VERSION | 0x1A   | Script version v1          |
| 0x01   | FLAGS   | 0x00   | Reserved                   |
| 0x02   | DELAY   | 0x0000 | Pre-execution delay (0ms)  |
| 0x04   | LENGTH  | 0x0021 | Bytecode length (33 bytes) |
| 0x06   | CRC16   | 0x6FB4 | Checksum                   |

**Complete Payload:**

```
Header: 1A 00 00 00 21 00 B4 6F
Payload: 07 08 15 01 F4 01 08 04 63 61 6C 63 05 28 01 E8 03 06 04 02 05 1F 08 01 2B 06 03 02 05 24 05 28 00
```

---

## Implementation Notes

**USB HID Constraints:**

- 6-key rollover (6KRO) maximum
- Modifier keys do not count toward 6-key limit
- Reports sent automatically on state change

**Timing Considerations:**

- DELAY precision: ±5ms typical
- USB report interval: 8ms (125 Hz)
- Minimum reliable inter-key delay: 10-20ms for compatibility

## Future Improvements

### New Opcodes

- **WAIT_LED**: Pause execution until a keyboard LED changes state (Caps Lock, Num Lock, Scroll Lock), enabling
  synchronization with OS responses.
- **IF_LED**: Conditional execution based on LED state, allowing scripts to branch depending on current Caps Lock or Num
  Lock status.
- **RANDOM_DELAY**: Pause for a random duration between min and max values, useful for human-like typing simulation and
  anti-detection scenarios.

### Opcode Enhancements

- **STRING with inter-character delay**: Add optional delay parameter to `STRING` opcode to simulate human typing
  speed (delay in 10ms increments).
- **DELAY resolution change**: Modify `DELAY` to use 10ms units instead of 1ms, extending maximum delay from ~65 seconds
  to ~10.9 minutes while maintaining 2-byte parameter.

### Script Flags

- **Loop mode flag**: Script restarts automatically after `END`, useful for continuous input patterns (e.g., keep-alive
  keystrokes, repeating macros).
- **LED initialization flags**: Set initial state of Caps Lock, Num Lock, and Scroll Lock at script start (ON, OFF,
  TOGGLE, or IGNORE), ensuring consistent keyboard state regardless of initial conditions.

## References

- [USB HID Keyboard scan codes](https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2)
- [CRC-16-CCITT Calculator](https://crccalc.com/?crc=&method=CRC-16/CCITT-FALSE&datatype=hex&outtype=hex)

---

## Changelog

### v1.0 (2025-02-07)

- Initial specification release
- Payload format version identifier `0x1A`
- Defined 8 opcodes: END, DELAY, KEY_DOWN, KEY_UP, MOD, TAP, COMBO, STRING
- Established three-layer architecture (primitives, common, shortcuts)