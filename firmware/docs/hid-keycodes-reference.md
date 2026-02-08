# USB HID Keyboard Keycodes Reference

A complete reference of USB HID keycodes as defined in the USB HID Usage Tables specification.

---

## Overview

USB HID (Human Interface Device) keycodes are standardized codes that represent keyboard keys. These codes are defined
in the USB HID Usage Tables specification and are universal across all USB keyboards.

**Important notes:**

1. **Keyboard Layout**: These keycodes represent US QWERTY layout. Keycodes represent physical keys, not characters;
   therefore, other layouts may produce different characters for the same keycode.
2. **6KRO Limit**: Standard USB HID keyboards support up to 6 simultaneous keycodes (plus modifiers). This is known as
   6-Key Rollover.
3. **Reserved Keycodes**: The valid keycode range for standard keys is 0x00-0x65. These codes handle system states and
   errors:
    - 0x00: No key pressed / reserved
    - 0x01: Error rollover
    - 0x02: POST fail
    - 0x03: Undefined error
4. **Modifier Keycodes**: While modifiers have physical keycodes (0xE0-0xE7), they are typically handled via the
   modifier byte in the HID report, not as regular keycodes.

---

## Letters

| Keycode | Key | Keycode | Key |
|---------|-----|---------|-----|
| 0x04    | A   | 0x11    | N   |
| 0x05    | B   | 0x12    | O   |
| 0x06    | C   | 0x13    | P   |
| 0x07    | D   | 0x14    | Q   |
| 0x08    | E   | 0x15    | R   |
| 0x09    | F   | 0x16    | S   |
| 0x0A    | G   | 0x17    | T   |
| 0x0B    | H   | 0x18    | U   |
| 0x0C    | I   | 0x19    | V   |
| 0x0D    | J   | 0x1A    | W   |
| 0x0E    | K   | 0x1B    | X   |
| 0x0F    | L   | 0x1C    | Y   |
| 0x10    | M   | 0x1D    | Z   |

**Note:** These keycodes produce lowercase letters by default. Combine with Shift modifier (0x02) for uppercase.

---

## Numbers (Top Row)

| Keycode | Key | With Shift |
|---------|-----|------------|
| 0x1E    | 1   | !          |
| 0x1F    | 2   | @          |
| 0x20    | 3   | #          |
| 0x21    | 4   | $          |
| 0x22    | 5   | %          |
| 0x23    | 6   | ^          |
| 0x24    | 7   | &          |
| 0x25    | 8   | *          |
| 0x26    | 9   | (          |
| 0x27    | 0   | )          |

---

## Special Keys

| Keycode | Key       | Description     |
|---------|-----------|-----------------|
| 0x28    | Enter     | Return / Enter  |
| 0x29    | Escape    | Esc             |
| 0x2A    | Backspace | Delete backward |
| 0x2B    | Tab       | Horizontal tab  |
| 0x2C    | Space     | Spacebar        |

---

## Punctuation and Symbols

| Keycode | Key | With Shift      |
|---------|-----|-----------------|
| 0x2D    | -   | _ (underscore)  |
| 0x2E    | =   | +               |
| 0x2F    | [   | {               |
| 0x30    | ]   | }               |
| 0x31    | \   | \| (pipe)       |
| 0x32    | #   | ~ (non-US)      |
| 0x33    | ;   | :               |
| 0x34    | '   | "               |
| 0x35    | `   | ~ (grave/tilde) |
| 0x36    | ,   | <               |
| 0x37    | .   | \>              |
| 0x38    | /   | ?               |

---

## Lock Keys

| Keycode | Key         | Description        |
|---------|-------------|--------------------|
| 0x39    | Caps Lock   | Toggle caps lock   |
| 0x47    | Scroll Lock | Toggle scroll lock |
| 0x53    | Num Lock    | Toggle num lock    |

---

## Function Keys

| Keycode | Key | Keycode | Key |
|---------|-----|---------|-----|
| 0x3A    | F1  | 0x40    | F7  |
| 0x3B    | F2  | 0x41    | F8  |
| 0x3C    | F3  | 0x42    | F9  |
| 0x3D    | F4  | 0x43    | F10 |
| 0x3E    | F5  | 0x44    | F11 |
| 0x3F    | F6  | 0x45    | F12 |

### Extended Function Keys

| Keycode | Key | Keycode | Key |
|---------|-----|---------|-----|
| 0x68    | F13 | 0x6C    | F17 |
| 0x69    | F14 | 0x6D    | F18 |
| 0x6A    | F15 | 0x6E    | F19 |
| 0x6B    | F16 | 0x6F    | F20 |

---

## Navigation Keys

| Keycode | Key          | Description          |
|---------|--------------|----------------------|
| 0x46    | Print Screen | PrtSc / SysRq        |
| 0x48    | Pause        | Pause / Break        |
| 0x49    | Insert       | Ins                  |
| 0x4A    | Home         | Home                 |
| 0x4B    | Page Up      | PgUp                 |
| 0x4C    | Delete       | Del (forward delete) |
| 0x4D    | End          | End                  |
| 0x4E    | Page Down    | PgDn                 |

---

## Arrow Keys

| Keycode | Key | Direction   |
|---------|-----|-------------|
| 0x4F    | →   | Right Arrow |
| 0x50    | ←   | Left Arrow  |
| 0x51    | ↓   | Down Arrow  |
| 0x52    | ↑   | Up Arrow    |

---

## Numeric Keypad

### Keypad Numbers

| Keycode | Key      | With Num Lock OFF |
|---------|----------|-------------------|
| 0x59    | Keypad 1 | End               |
| 0x5A    | Keypad 2 | Down Arrow        |
| 0x5B    | Keypad 3 | Page Down         |
| 0x5C    | Keypad 4 | Left Arrow        |
| 0x5D    | Keypad 5 | (none)            |
| 0x5E    | Keypad 6 | Right Arrow       |
| 0x5F    | Keypad 7 | Home              |
| 0x60    | Keypad 8 | Up Arrow          |
| 0x61    | Keypad 9 | Page Up           |
| 0x62    | Keypad 0 | Insert            |
| 0x63    | Keypad . | Delete            |

### Keypad Operators

| Keycode | Key   | Description     |
|---------|-------|-----------------|
| 0x54    | /     | Keypad divide   |
| 0x55    | *     | Keypad multiply |
| 0x56    | -     | Keypad subtract |
| 0x57    | +     | Keypad add      |
| 0x58    | Enter | Keypad enter    |
| 0x67    | =     | Keypad equals   |

---

## Modifier Keys Bitmap

Modifiers are **not** keycodes. They are represented as a bitmap in byte 0 of the HID report.

| Bit | Mask | Modifier    | Description             |
|-----|------|-------------|-------------------------|
| 0   | 0x01 | Left Ctrl   | Left Control            |
| 1   | 0x02 | Left Shift  | Left Shift              |
| 2   | 0x04 | Left Alt    | Left Alt / Option       |
| 3   | 0x08 | Left GUI    | Left Windows / Command  |
| 4   | 0x10 | Right Ctrl  | Right Control           |
| 5   | 0x20 | Right Shift | Right Shift             |
| 6   | 0x40 | Right Alt   | Right Alt / AltGr       |
| 7   | 0x80 | Right GUI   | Right Windows / Command |

### Common Modifier Combinations

| Mask | Combination        |
|------|--------------------|
| 0x01 | Ctrl               |
| 0x02 | Shift              |
| 0x03 | Ctrl + Shift       |
| 0x04 | Alt                |
| 0x05 | Ctrl + Alt         |
| 0x06 | Shift + Alt        |
| 0x07 | Ctrl + Shift + Alt |
| 0x08 | GUI (Win/Cmd)      |
| 0x09 | Ctrl + GUI         |
| 0x0A | Shift + GUI        |
| 0x0C | Alt + GUI          |

---

## Application Keys

| Keycode | Key         | Description      |
|---------|-------------|------------------|
| 0x65    | Application | Context menu key |
| 0x66    | Power       | System power     |
| 0x74    | Execute     |                  |
| 0x75    | Help        |                  |
| 0x76    | Menu        |                  |
| 0x77    | Select      |                  |
| 0x78    | Stop        |                  |
| 0x79    | Again       | Redo             |
| 0x7A    | Undo        |                  |
| 0x7B    | Cut         |                  |
| 0x7C    | Copy        |                  |
| 0x7D    | Paste       |                  |
| 0x7E    | Find        |                  |
| 0x7F    | Mute        |                  |
| 0x80    | Volume Up   |                  |
| 0x81    | Volume Down |                  |

---

## References

- [USB HID Usage Tables](https://usb.org/document-library/hid-usage-tables-17): Section *10 Keyboard/Keypad Page (0x07)*