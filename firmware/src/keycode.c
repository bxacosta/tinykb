/**
 * keycode.c - ASCII to USB HID keycode conversion
 *
 * Lookup table for US keyboard layout stored in PROGMEM.
 */

#include "keycode.h"

#include <avr/pgmspace.h>

/* -------------------------------------------------------------------------- */
/* Constants                                                                  */
/* -------------------------------------------------------------------------- */

#define SHIFT_FLAG   0x80
#define KEYCODE_MASK 0x7F

#define TABLE_OFFSET 8
#define TABLE_SIZE   119   /* ASCII 8-126 */

/* -------------------------------------------------------------------------- */
/* Private                                                                    */
/* -------------------------------------------------------------------------- */

/* Lookup Table */

static const uint8_t PROGMEM ascii_to_keycode[TABLE_SIZE] = {
    /* 008 */ 0x2A,                /* Backspace */
    /* 009 */ 0x2B,                /* Tab       */
    /* 010 */ 0x28,                /* Enter     */
    /* 011 */ 0x00,
    /* 012 */ 0x00,
    /* 013 */ 0x00,
    /* 014 */ 0x00,
    /* 015 */ 0x00,
    /* 016 */ 0x00,
    /* 017 */ 0x00,
    /* 018 */ 0x00,
    /* 019 */ 0x00,
    /* 020 */ 0x00,
    /* 021 */ 0x00,
    /* 022 */ 0x00,
    /* 023 */ 0x00,
    /* 024 */ 0x00,
    /* 025 */ 0x00,
    /* 026 */ 0x00,
    /* 027 */ 0x29,                /* Escape    */
    /* 028 */ 0x00,
    /* 029 */ 0x00,
    /* 030 */ 0x00,
    /* 031 */ 0x00,
    /* 032 */ 0x2C,                /* Space     */
    /* 033 */ 0x1E | SHIFT_FLAG,   /* !         */
    /* 034 */ 0x34 | SHIFT_FLAG,   /* "         */
    /* 035 */ 0x20 | SHIFT_FLAG,   /* #         */
    /* 036 */ 0x21 | SHIFT_FLAG,   /* $         */
    /* 037 */ 0x22 | SHIFT_FLAG,   /* %         */
    /* 038 */ 0x24 | SHIFT_FLAG,   /* &         */
    /* 039 */ 0x34,                /* '         */
    /* 040 */ 0x26 | SHIFT_FLAG,   /* (         */
    /* 041 */ 0x27 | SHIFT_FLAG,   /* )         */
    /* 042 */ 0x25 | SHIFT_FLAG,   /* *         */
    /* 043 */ 0x2E | SHIFT_FLAG,   /* +         */
    /* 044 */ 0x36,                /* ,         */
    /* 045 */ 0x2D,                /* -         */
    /* 046 */ 0x37,                /* .         */
    /* 047 */ 0x38,                /* /         */
    /* 048 */ 0x27,                /* 0         */
    /* 049 */ 0x1E,                /* 1         */
    /* 050 */ 0x1F,                /* 2         */
    /* 051 */ 0x20,                /* 3         */
    /* 052 */ 0x21,                /* 4         */
    /* 053 */ 0x22,                /* 5         */
    /* 054 */ 0x23,                /* 6         */
    /* 055 */ 0x24,                /* 7         */
    /* 056 */ 0x25,                /* 8         */
    /* 057 */ 0x26,                /* 9         */
    /* 058 */ 0x33 | SHIFT_FLAG,   /* :         */
    /* 059 */ 0x33,                /* ;         */
    /* 060 */ 0x36 | SHIFT_FLAG,   /* <         */
    /* 061 */ 0x2E,                /* =         */
    /* 062 */ 0x37 | SHIFT_FLAG,   /* >         */
    /* 063 */ 0x38 | SHIFT_FLAG,   /* ?         */
    /* 064 */ 0x1F | SHIFT_FLAG,   /* @         */
    /* 065 */ 0x04 | SHIFT_FLAG,   /* A         */
    /* 066 */ 0x05 | SHIFT_FLAG,   /* B         */
    /* 067 */ 0x06 | SHIFT_FLAG,   /* C         */
    /* 068 */ 0x07 | SHIFT_FLAG,   /* D         */
    /* 069 */ 0x08 | SHIFT_FLAG,   /* E         */
    /* 070 */ 0x09 | SHIFT_FLAG,   /* F         */
    /* 071 */ 0x0A | SHIFT_FLAG,   /* G         */
    /* 072 */ 0x0B | SHIFT_FLAG,   /* H         */
    /* 073 */ 0x0C | SHIFT_FLAG,   /* I         */
    /* 074 */ 0x0D | SHIFT_FLAG,   /* J         */
    /* 075 */ 0x0E | SHIFT_FLAG,   /* K         */
    /* 076 */ 0x0F | SHIFT_FLAG,   /* L         */
    /* 077 */ 0x10 | SHIFT_FLAG,   /* M         */
    /* 078 */ 0x11 | SHIFT_FLAG,   /* N         */
    /* 079 */ 0x12 | SHIFT_FLAG,   /* O         */
    /* 080 */ 0x13 | SHIFT_FLAG,   /* P         */
    /* 081 */ 0x14 | SHIFT_FLAG,   /* Q         */
    /* 082 */ 0x15 | SHIFT_FLAG,   /* R         */
    /* 083 */ 0x16 | SHIFT_FLAG,   /* S         */
    /* 084 */ 0x17 | SHIFT_FLAG,   /* T         */
    /* 085 */ 0x18 | SHIFT_FLAG,   /* U         */
    /* 086 */ 0x19 | SHIFT_FLAG,   /* V         */
    /* 087 */ 0x1A | SHIFT_FLAG,   /* W         */
    /* 088 */ 0x1B | SHIFT_FLAG,   /* X         */
    /* 089 */ 0x1C | SHIFT_FLAG,   /* Y         */
    /* 090 */ 0x1D | SHIFT_FLAG,   /* Z         */
    /* 091 */ 0x2F,                /* [         */
    /* 092 */ 0x31,                /* \         */
    /* 093 */ 0x30,                /* ]         */
    /* 094 */ 0x23 | SHIFT_FLAG,   /* ^         */
    /* 095 */ 0x2D | SHIFT_FLAG,   /* _         */
    /* 096 */ 0x35,                /* `         */
    /* 097 */ 0x04,                /* a         */
    /* 098 */ 0x05,                /* b         */
    /* 099 */ 0x06,                /* c         */
    /* 100 */ 0x07,                /* d         */
    /* 101 */ 0x08,                /* e         */
    /* 102 */ 0x09,                /* f         */
    /* 103 */ 0x0A,                /* g         */
    /* 104 */ 0x0B,                /* h         */
    /* 105 */ 0x0C,                /* i         */
    /* 106 */ 0x0D,                /* j         */
    /* 107 */ 0x0E,                /* k         */
    /* 108 */ 0x0F,                /* l         */
    /* 109 */ 0x10,                /* m         */
    /* 110 */ 0x11,                /* n         */
    /* 111 */ 0x12,                /* o         */
    /* 112 */ 0x13,                /* p         */
    /* 113 */ 0x14,                /* q         */
    /* 114 */ 0x15,                /* r         */
    /* 115 */ 0x16,                /* s         */
    /* 116 */ 0x17,                /* t         */
    /* 117 */ 0x18,                /* u         */
    /* 118 */ 0x19,                /* v         */
    /* 119 */ 0x1A,                /* w         */
    /* 120 */ 0x1B,                /* x         */
    /* 121 */ 0x1C,                /* y         */
    /* 122 */ 0x1D,                /* z         */
    /* 123 */ 0x2F | SHIFT_FLAG,   /* {         */
    /* 124 */ 0x31 | SHIFT_FLAG,   /* |         */
    /* 125 */ 0x30 | SHIFT_FLAG,   /* }         */
    /* 126 */ 0x35 | SHIFT_FLAG    /* ~         */
};

/* -------------------------------------------------------------------------- */
/* Public                                                                     */
/* -------------------------------------------------------------------------- */

/* Conversion */

keycode_result_t keycode_from_ascii(char c) {
    keycode_result_t result = {0, 0};
    uint8_t index = (uint8_t)c - TABLE_OFFSET;

    if (index >= TABLE_SIZE) {
        return result;
    }

    uint8_t entry = pgm_read_byte(&ascii_to_keycode[index]);

    if (entry == 0) {
        return result;
    }

    result.keycode = entry & KEYCODE_MASK;
    result.modifiers = (entry & SHIFT_FLAG) ? MOD_SHIFT : MOD_NONE;

    return result;
}
