/**
 * script_engine.h - Bytecode interpreter for keyboard scripts
 *
 * Executes scripts stored in EEPROM. Call engine_tick() frequently from main loop.
 */

#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

/* Opcodes */

#define OP_END      0x00
#define OP_DELAY    0x01
#define OP_KEY_DOWN 0x02
#define OP_KEY_UP   0x03
#define OP_MOD      0x04
#define OP_TAP      0x05
#define OP_REPEAT   0x06
#define OP_COMBO    0x07
#define OP_STRING   0x08

/* Types */

typedef enum {
    ENGINE_IDLE,
    ENGINE_RUNNING,
    ENGINE_DELAYING,
    ENGINE_FINISHED,
    ENGINE_ERROR
} engine_state_t;

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/* Lifecycle */

void engine_init(void);
void engine_start(void);
void engine_stop(void);

/* Execution */

engine_state_t engine_tick(void);

/* Status */

engine_state_t engine_get_state(void);
bool engine_is_running(void);

#endif
