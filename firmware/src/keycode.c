/**
 * keycode.c - ASCII to USB HID keycode conversion
 */

#include "keycode.h"
#include <avr/pgmspace.h>

/* Constants */

#define SHIFT_FLAG 0x80
#define KEYCODE_MASK 0x7F

/*
 * ASCII to keycode lookup table (US keyboard layout)
 * Format: bit 7 = requires Shift, bits 0-6 = keycode
 * Table covers ASCII 8-126 (index = ascii - 8)
 */
#define TABLE_OFFSET 8
#define TABLE_SIZE   119  /* 126 - 8 + 1 */

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* Lookup table */

static const uint8_t PROGMEM ascii_to_keycode[TABLE_SIZE] = {
    /* ASCII   8 */ 0x2A,              /* Backspace */
    /* ASCII   9 */ 0x2B,              /* Tab */
    /* ASCII  10 */ 0x28,              /* Enter (newline) */
    /* ASCII  11 */ 0x00,
    /* ASCII  12 */ 0x00,
    /* ASCII  13 */ 0x00,
    /* ASCII  14 */ 0x00,
    /* ASCII  15 */ 0x00,
    /* ASCII  16 */ 0x00,
    /* ASCII  17 */ 0x00,
    /* ASCII  18 */ 0x00,
    /* ASCII  19 */ 0x00,
    /* ASCII  20 */ 0x00,
    /* ASCII  21 */ 0x00,
    /* ASCII  22 */ 0x00,
    /* ASCII  23 */ 0x00,
    /* ASCII  24 */ 0x00,
    /* ASCII  25 */ 0x00,
    /* ASCII  26 */ 0x00,
    /* ASCII  27 */ 0x29,              /* Escape */
    /* ASCII  28 */ 0x00,
    /* ASCII  29 */ 0x00,
    /* ASCII  30 */ 0x00,
    /* ASCII  31 */ 0x00,
    /* ASCII  32 */ 0x2C,              /* Space */
    /* ASCII  33 */ 0x1E | SHIFT_FLAG, /* ! */
    /* ASCII  34 */ 0x34 | SHIFT_FLAG, /* " */
    /* ASCII  35 */ 0x20 | SHIFT_FLAG, /* # */
    /* ASCII  36 */ 0x21 | SHIFT_FLAG, /* $ */
    /* ASCII  37 */ 0x22 | SHIFT_FLAG, /* % */
    /* ASCII  38 */ 0x24 | SHIFT_FLAG, /* & */
    /* ASCII  39 */ 0x34,              /* ' */
    /* ASCII  40 */ 0x26 | SHIFT_FLAG, /* ( */
    /* ASCII  41 */ 0x27 | SHIFT_FLAG, /* ) */
    /* ASCII  42 */ 0x25 | SHIFT_FLAG, /* * */
    /* ASCII  43 */ 0x2E | SHIFT_FLAG, /* + */
    /* ASCII  44 */ 0x36,              /* , */
    /* ASCII  45 */ 0x2D,              /* - */
    /* ASCII  46 */ 0x37,              /* . */
    /* ASCII  47 */ 0x38,              /* / */
    /* ASCII  48 */ 0x27,              /* 0 */
    /* ASCII  49 */ 0x1E,              /* 1 */
    /* ASCII  50 */ 0x1F,              /* 2 */
    /* ASCII  51 */ 0x20,              /* 3 */
    /* ASCII  52 */ 0x21,              /* 4 */
    /* ASCII  53 */ 0x22,              /* 5 */
    /* ASCII  54 */ 0x23,              /* 6 */
    /* ASCII  55 */ 0x24,              /* 7 */
    /* ASCII  56 */ 0x25,              /* 8 */
    /* ASCII  57 */ 0x26,              /* 9 */
    /* ASCII  58 */ 0x33 | SHIFT_FLAG, /* : */
    /* ASCII  59 */ 0x33,              /* ; */
    /* ASCII  60 */ 0x36 | SHIFT_FLAG, /* < */
    /* ASCII  61 */ 0x2E,              /* = */
    /* ASCII  62 */ 0x37 | SHIFT_FLAG, /* > */
    /* ASCII  63 */ 0x38 | SHIFT_FLAG, /* ? */
    /* ASCII  64 */ 0x1F | SHIFT_FLAG, /* @ */
    /* ASCII  65 */ 0x04 | SHIFT_FLAG, /* A */
    /* ASCII  66 */ 0x05 | SHIFT_FLAG, /* B */
    /* ASCII  67 */ 0x06 | SHIFT_FLAG, /* C */
    /* ASCII  68 */ 0x07 | SHIFT_FLAG, /* D */
    /* ASCII  69 */ 0x08 | SHIFT_FLAG, /* E */
    /* ASCII  70 */ 0x09 | SHIFT_FLAG, /* F */
    /* ASCII  71 */ 0x0A | SHIFT_FLAG, /* G */
    /* ASCII  72 */ 0x0B | SHIFT_FLAG, /* H */
    /* ASCII  73 */ 0x0C | SHIFT_FLAG, /* I */
    /* ASCII  74 */ 0x0D | SHIFT_FLAG, /* J */
    /* ASCII  75 */ 0x0E | SHIFT_FLAG, /* K */
    /* ASCII  76 */ 0x0F | SHIFT_FLAG, /* L */
    /* ASCII  77 */ 0x10 | SHIFT_FLAG, /* M */
    /* ASCII  78 */ 0x11 | SHIFT_FLAG, /* N */
    /* ASCII  79 */ 0x12 | SHIFT_FLAG, /* O */
    /* ASCII  80 */ 0x13 | SHIFT_FLAG, /* P */
    /* ASCII  81 */ 0x14 | SHIFT_FLAG, /* Q */
    /* ASCII  82 */ 0x15 | SHIFT_FLAG, /* R */
    /* ASCII  83 */ 0x16 | SHIFT_FLAG, /* S */
    /* ASCII  84 */ 0x17 | SHIFT_FLAG, /* T */
    /* ASCII  85 */ 0x18 | SHIFT_FLAG, /* U */
    /* ASCII  86 */ 0x19 | SHIFT_FLAG, /* V */
    /* ASCII  87 */ 0x1A | SHIFT_FLAG, /* W */
    /* ASCII  88 */ 0x1B | SHIFT_FLAG, /* X */
    /* ASCII  89 */ 0x1C | SHIFT_FLAG, /* Y */
    /* ASCII  90 */ 0x1D | SHIFT_FLAG, /* Z */
    /* ASCII  91 */ 0x2F,              /* [ */
    /* ASCII  92 */ 0x31,              /* \ */
    /* ASCII  93 */ 0x30,              /* ] */
    /* ASCII  94 */ 0x23 | SHIFT_FLAG, /* ^ */
    /* ASCII  95 */ 0x2D | SHIFT_FLAG, /* _ */
    /* ASCII  96 */ 0x35,              /* ` */
    /* ASCII  97 */ 0x04,              /* a */
    /* ASCII  98 */ 0x05,              /* b */
    /* ASCII  99 */ 0x06,              /* c */
    /* ASCII 100 */ 0x07,              /* d */
    /* ASCII 101 */ 0x08,              /* e */
    /* ASCII 102 */ 0x09,              /* f */
    /* ASCII 103 */ 0x0A,              /* g */
    /* ASCII 104 */ 0x0B,              /* h */
    /* ASCII 105 */ 0x0C,              /* i */
    /* ASCII 106 */ 0x0D,              /* j */
    /* ASCII 107 */ 0x0E,              /* k */
    /* ASCII 108 */ 0x0F,              /* l */
    /* ASCII 109 */ 0x10,              /* m */
    /* ASCII 110 */ 0x11,              /* n */
    /* ASCII 111 */ 0x12,              /* o */
    /* ASCII 112 */ 0x13,              /* p */
    /* ASCII 113 */ 0x14,              /* q */
    /* ASCII 114 */ 0x15,              /* r */
    /* ASCII 115 */ 0x16,              /* s */
    /* ASCII 116 */ 0x17,              /* t */
    /* ASCII 117 */ 0x18,              /* u */
    /* ASCII 118 */ 0x19,              /* v */
    /* ASCII 119 */ 0x1A,              /* w */
    /* ASCII 120 */ 0x1B,              /* x */
    /* ASCII 121 */ 0x1C,              /* y */
    /* ASCII 122 */ 0x1D,              /* z */
    /* ASCII 123 */ 0x2F | SHIFT_FLAG, /* { */
    /* ASCII 124 */ 0x31 | SHIFT_FLAG, /* | */
    /* ASCII 125 */ 0x30 | SHIFT_FLAG, /* } */
    /* ASCII 126 */ 0x35 | SHIFT_FLAG  /* ~ */
};

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Conversion */

keycode_result_t keycode_from_ascii(char c) {
    keycode_result_t result = {0, 0};
    uint8_t index = (uint8_t)c - TABLE_OFFSET;

    if (index >= TABLE_SIZE) {
        return result;  /* Out of range */
    }

    uint8_t entry = pgm_read_byte(&ascii_to_keycode[index]);

    if (entry == 0) {
        return result;  /* Unsupported character */
    }

    result.keycode = entry & KEYCODE_MASK;
    result.modifiers = (entry & SHIFT_FLAG) ? MOD_SHIFT : MOD_NONE;

    return result;
}
