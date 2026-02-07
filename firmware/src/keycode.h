/**
 * keycode.h - ASCII to USB HID keycode conversion
 *
 * Converts ASCII characters to USB HID keycodes for US keyboard layout.
 * Provides constants for modifier keys and special keys.
 */

#ifndef KEYCODE_H
#define KEYCODE_H

#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

/* Modifier masks (byte 0 of HID report) */
#define MOD_NONE        0x00
#define MOD_CTRL_LEFT   0x01
#define MOD_SHIFT_LEFT  0x02
#define MOD_ALT_LEFT    0x04
#define MOD_GUI_LEFT    0x08
#define MOD_CTRL_RIGHT  0x10
#define MOD_SHIFT_RIGHT 0x20
#define MOD_ALT_RIGHT   0x40
#define MOD_GUI_RIGHT   0x80

/* Common modifier aliases */
#define MOD_CTRL        MOD_CTRL_LEFT
#define MOD_SHIFT       MOD_SHIFT_LEFT
#define MOD_ALT         MOD_ALT_LEFT
#define MOD_GUI         MOD_GUI_LEFT

/* Special keycodes */
#define KEY_ENTER       0x28
#define KEY_ESC         0x29
#define KEY_BACKSPACE   0x2A
#define KEY_TAB         0x2B
#define KEY_SPACE       0x2C
#define KEY_CAPS_LOCK   0x39

/* Function keys */
#define KEY_F1          0x3A
#define KEY_F2          0x3B
#define KEY_F3          0x3C
#define KEY_F4          0x3D
#define KEY_F5          0x3E
#define KEY_F6          0x3F
#define KEY_F7          0x40
#define KEY_F8          0x41
#define KEY_F9          0x42
#define KEY_F10         0x43
#define KEY_F11         0x44
#define KEY_F12         0x45

/* Navigation keys */
#define KEY_INSERT      0x49
#define KEY_HOME        0x4A
#define KEY_PAGE_UP     0x4B
#define KEY_DELETE      0x4C
#define KEY_END         0x4D
#define KEY_PAGE_DOWN   0x4E
#define KEY_ARROW_RIGHT 0x4F
#define KEY_ARROW_LEFT  0x50
#define KEY_ARROW_DOWN  0x51
#define KEY_ARROW_UP    0x52

/* -------------------------------------------------------------------------- */
/* Types                                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t keycode;
    uint8_t modifiers;
} keycode_result_t;

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Conversion */

keycode_result_t keycode_from_ascii(char c);

#endif /* KEYCODE_H */
