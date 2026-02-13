# TinyKB Firmware Architecture

## Overview

TinyKB is an ATtiny85-based USB keyboard that executes stored keystroke scripts. Users upload scripts via a web browser
using WebHID (no drivers needed), then the device operates as a standard USB keyboard.

The firmware implements a single binary with two operating modes:

| Mode        | USB Class                | Purpose                       |
|-------------|--------------------------|-------------------------------|
| Programming | Raw HID (Usage 0xFF00)   | Accept scripts via WebHID     |
| Keyboard    | Boot Protocol HID (0x01) | Execute scripts as keystrokes |

Mode is determined at boot using MCUSR/GPIOR0 watchdog reset detection and transitions use a watchdog reset.

## Design Decisions

### Dual-Mode USB Architecture

TinyKB employs a sequential dual-mode USB architecture to satisfy conflicting requirements: universal keyboard
compatibility (Boot Protocol HID) and driverless browser access (WebHID). Since modern browser security policies block
WebHID access to Boot Protocol keyboards, the device must alternate between two distinct USB identities.

1. **Programming Mode (Raw HID 0xFF00):** Exposes a vendor-specific interface for WebHID script uploads.
2. **Keyboard Mode (Boot Protocol HID 0x01):** Presents a standard keyboard interface compatible with BIOS/UEFI.

#### Technical Rationale

The sequential approach is selected over a composite device architecture due to specific hardware and software
constraints:

- **V-USB Stack Optimization:** V-USB is designed primarily for single-interface devices. A sequential approach avoids
  the complexity and overhead of multiplexing endpoints in software.
- **Resource Constraints:** The ATtiny85 has limited RAM (512 bytes). Managing multiple simultaneous HID report buffers
  and state machines would exceed available resources.
- **State Reliability:** Switching modes via a Watchdog Reset guarantees a clean system state, eliminating potential USB
  descriptor caching issues or driver conflicts on the host.

### Deployment and Bootloader Integration

The firmware is designed to be compatible with various deployment scenarios, each offering different trade-offs in terms
of transition speed and available storage.

| Configuration   | Bootloader              | Transition Speed | Available Flash | Notes                                                               |
|-----------------|-------------------------|------------------|-----------------|---------------------------------------------------------------------|
| **Development** | Micronucleus (Standard) | ~5.4 seconds     | ~6 KB           | Default used by Digispark. 5s bootloader wait on every reset.       |
| **Optimized**   | Micronucleus (Modified) | ~0.4 seconds     | ~6 KB           | Uses `ENTRY_POWER_ON` to skip bootloader on watchdog resets.        |
| **Standalone**  | None (ISP Direct)       | ~0.4 seconds     | ~8 KB           | Maximum performance and space. Requires ISP programmer for updates. |

The transition from Programming to Keyboard mode is triggered via a **Watchdog Reset**. This mechanism is reliable and
avoids the need for persistent flags in EEPROM, preserving EEPROM longevity and maximizing space for user scripts.

## Device Flow

```
Power On
    |
    v
Micronucleus Bootloader (5 seconds)
    |
    v
TinyKB Firmware Start
    |
    v
+---------------------------+
|    PROGRAMMING MODE       |  <-- WebHID connects here
|    (Raw HID 0xFF00)       |      Program scripts via HID reports
|    LED on                 |
|    5 second timeout       |
+---------------------------+
    |
    | Timeout (no host activity) OR CMD_EXIT
    v
+---------------------------+
|    Watchdog Reset         |
+---------------------------+
    |
    v
+---------------------------+
|    KEYBOARD MODE          |  <-- Normal keyboard operation
|    (Boot Protocol HID)    |      Execute stored scripts
|    LED off                |
+---------------------------+
```

**Programming mode timeout:** If no HID report is received within 5 seconds, the device transitions to keyboard mode.
Once any report is received (activity detected), the timeout is permanently disabled and the device stays in programming
mode until `CMD_EXIT` is sent.

## Module Architecture

### Layer Diagram

```
+------------------------------------------------------------------+
|                            main.c                                |
|                      (Orchestrator only)                         |
+------------------------------------------------------------------+
        |                      |                       |
        v                      v                       v
+----------------+    +----------------+    +----------------------+
|  device_mode   |--->|    usb_core    |    |   eeprom_storage     |
|  (mode state)  |    | (lifecycle)    |    |  (script storage)    |
+----------------+    +----------------+    +----------------------+
        |                      |                       ^  
        v                      v                       |
+------------------------------------------------------------------+
|                        usb_dispatcher                            |
|              (V-USB callbacks, request routing)                  |
+------------------------------------------------------------------+
                               |
                               v
+--------------------+                      +----------------------+
|   usb_rawhid       |                      |    usb_keyboard      |
| (Programming Mode) |                      |   (Keyboard Mode)    |
| Raw HID reports    |                      |  Boot Protocol HID   |
+--------------------+                      +----------------------+
        |                                              |     
        v                                              v     
+--------------------+                      +----------------------+
|   hid_protocol     |                      |   script_engine      |
| (command handlers) |                      |  (bytecode interp)   |
+--------------------+                      +----------------------+
                                                       |     
                                                       v     
                                            +----------------------+
                                            |      keycode         |
                                            |  (ASCII to HID)      |
                                            +----------------------+

+------------------------------------------------------------------+
|                           Utility Modules                        |
+------------------------------------------------------------------+
|      timer     |      crc16     |   oscillator   |      led      |
+---------------------+-----------------------+--------------------+
```

### Dependency Graph

```
Level 0 (No project dependencies):
├── config.h
├── timer.c/h
├── keycode.c/h
├── crc16.c/h
├── led.c/h
├── oscillator.c/h
└── usb_core.c/h        -> (V-USB)

Level 1 (Depends on Level 0):
├── eeprom_storage.c/h  -> config.h, crc16.h
└── usb_keyboard.c/h    -> (V-USB)

Level 2 (Depends on Level 1):
├── device_mode.c/h     -> eeprom_storage.h, led.h, usb_core.h, usb_keyboard.h, usb_rawhid.h
├── hid_protocol.c/h    -> config.h, eeprom_storage.h, crc16.h
└── script_engine.c/h   -> config.h, eeprom_storage.h, keycode.h, timer.h

Level 3 (Depends on Level 2):
├── usb_rawhid.c/h      -> config.h, hid_protocol.h
├── usb_descriptors.c/h -> device_mode.h, config.h
└── usb_dispatcher.c/h  -> device_mode.h, usb_descriptors.h, usb_rawhid.h, usb_keyboard.h

Level 4 (Top level):
└── main.c              -> led.h, timer.h, eeprom_storage.h, device_mode.h
```

## File Structure

```
firmware/src/
|
|-- config.h                 # Centralized configuration constants
|-- main.c                   # Entry point, orchestrator
|
|-- Device Mode
|   |-- device_mode.c       # Mode state machine, USB init, mode loops
|   |-- device_mode.h
|
|-- USB Layer
|   |-- usb_core.c          # USB lifecycle (init/poll)
|   |-- usb_core.h
|   |-- usb_dispatcher.c    # V-USB callbacks dispatcher
|   |-- usb_dispatcher.h
|   |-- usb_descriptors.c   # Dynamic USB descriptors (PROGMEM)
|   |-- usb_descriptors.h
|   |-- usbconfig.h         # V-USB configuration
|
|-- Programming Mode
|   |-- usb_rawhid.c        # Raw HID setup/report handling
|   |-- usb_rawhid.h
|   |-- hid_protocol.c      # Command processing (WRITE, READ, etc.)
|   |-- hid_protocol.h
|
|-- Keyboard Mode
|   |-- usb_keyboard.c      # Boot Protocol HID keyboard
|   |-- usb_keyboard.h
|   |-- script_engine.c     # Bytecode interpreter
|   |-- script_engine.h
|   |-- keycode.c           # ASCII to keycode conversion
|   |-- keycode.h
|
|-- Storage
|   |-- eeprom_storage.c    # EEPROM read/write/validation
|   |-- eeprom_storage.h
|
|-- Utilities
|   |-- timer.c             # Hardware Timer1, millisecond resolution
|   |-- timer.h
|   |-- crc16.c             # CRC16-CCITT calculation
|   |-- crc16.h
|   |-- oscillator.c        # RC oscillator calibration
|   |-- oscillator.h
|   |-- led.c               # LED control (PB1)
|   |-- led.h
```

## Centralized Configuration (config.h)

All shared constants are defined in `config.h`. Module-specific constants remain in their respective headers.

### Design Principles

1. **Hardware constants** define physical limits (EEPROM size)
2. **Protocol constants** define communication parameters (report size)
3. **Derived constants** are calculated from base constants (max script size)
4. **Module-specific constants** remain in their headers (command codes, pin assignments)

### Constants

| Category     | Constant                    | Value  | Notes                              |
|--------------|-----------------------------|--------|------------------------------------|
| **Hardware** | `HW_EEPROM_SIZE`            | 512    | ATtiny85 EEPROM                    |
| **Protocol** | `PROTOCOL_REPORT_SIZE`      | 32     | HID report size                    |
| **Protocol** | `PROTOCOL_FIRMWARE_VERSION` | 0x01   | For STATUS response                |
| **Header**   | `STORAGE_HEADER_SIZE`       | 8      | Script header size                 |
| **Header**   | `STORAGE_PAYLOAD_VERSION`   | 0x1A   | Payload format version identifier  |
| **Header**   | `HEADER_OFFSET_*`           | 0-6    | VERSION, FLAGS, DELAY, LENGTH, CRC |
| **Derived**  | `STORAGE_EEPROM_SIZE`       | 512    | = `HW_EEPROM_SIZE`                 |
| **Derived**  | `STORAGE_SCRIPT_START`      | 8      | = `STORAGE_HEADER_SIZE`            |
| **Derived**  | `STORAGE_MAX_SCRIPT_SIZE`   | 504    | EEPROM - header                    |
| **Derived**  | `PROTOCOL_MAX_WRITE_DATA`   | 27     | Report size - overhead(5)          |
| **Derived**  | `PROTOCOL_MAX_READ_DATA`    | 29     | Report size - overhead(3)          |
| **Derived**  | `PROTOCOL_MAX_APPEND_DATA`  | 29     | Report size - overhead(3)          |
| **CRC**      | `CRC16_INIT`                | 0xFFFF | CRC-16-CCITT initial value         |
| **CRC**      | `CRC16_POLY`                | 0x1021 | CRC-16-CCITT polynomial            |

### Module-Specific Constants (NOT in config.h)

| Module              | Constants                                               | Reason                |
|---------------------|---------------------------------------------------------|-----------------------|
| `hid_protocol.h`    | `PROTOCOL_CMD_*`, `PROTOCOL_STATUS_*`, `PROTOCOL_OPT_*` | Command codes         |
| `led.h`             | `LED_PIN` (PB1)                                         | Hardware pin          |
| `usb_keyboard.h`    | `KEYBOARD_REPORT_SIZE`, `KEYBOARD_MAX_KEYS`             | Keyboard-specific     |
| `usb_descriptors.h` | `DESCRIPTOR_TYPE_*`                                     | USB descriptor types  |
| `script_engine.h`   | `OP_*` opcodes                                          | Bytecode-specific     |
| `keycode.h`         | `MOD_*`, `KEY_*`                                        | HID keycode constants |

## Module Specifications

### 1. main.c (Orchestrator)

**Purpose:** Entry point. Initializes modules and delegates execution. Contains no business logic.

```c
int main(void) {
    led_init();
    timer_init();
    storage_init();
    device_mode_init();
    device_mode_run();  /* Never returns */
    return 0;
}
```

Oscillator calibration is handled automatically by V-USB via `USB_RESET_HOOK` during USB enumeration.

**Dependencies:** `led.h`, `timer.h`, `eeprom_storage.h`, `device_mode.h`

### 2. device_mode.c/h (Mode State Machine)

**Purpose:** Manages mode detection, USB initialization, and runs the appropriate mode loop.

**Public API:**

```c
void device_mode_init(void);                    /* Detect mode, set LED */
void device_mode_run(void);                     /* Run mode loop (never returns) */
bool device_mode_is_programming(void);          /* Query current mode */
bool device_mode_is_keyboard(void);             /* Query current mode */
void device_mode_transition_to_keyboard(void);  /* Watchdog reset */
```

**Internal types and functions (private to .c):**

```c
typedef enum { DEVICE_MODE_PROGRAMMING, DEVICE_MODE_KEYBOARD } device_mode_t;

static device_mode_t determine_initial_mode(void);  /* Check MCUSR/GPIOR0 */
static void trigger_watchdog_reset(void);            /* wdt_enable + infinite loop */
static void init_usb(void);                          /* Shared USB disconnect/connect/init */
static void run_programming_loop(void);              /* Raw HID loop with 5s timeout */
static void run_keyboard_loop(void);                 /* Script execution loop */
```

**Mode detection logic:**

1. Read reset source from MCUSR (falls back to GPIOR0 if MCUSR is zero, which happens when Micronucleus
   bootloader clears MCUSR but saves it to GPIOR0 via `SAVE_MCUSR`)
2. Clear MCUSR and disable watchdog
3. If WDRF bit is set in reset source: return `DEVICE_MODE_KEYBOARD`
4. Otherwise: return `DEVICE_MODE_PROGRAMMING`

**USB initialization (`init_usb`):**

Shared between both modes. Pulls USB lines low, disconnects for 300ms, reconnects, calls `usbInit()`, enables
interrupts.

**Programming loop:**

1. `init_usb()` + `rawhid_init()`
2. Poll `usbPoll()` in a loop
3. Exit conditions: `rawhid_should_exit()` or timeout (5s with no activity)
4. `rawhid_had_activity()` returns a permanent flag — once true, timeout is disabled

**Keyboard loop:**

1. `init_usb()` + `keyboard_init()` + `engine_init()`
2. Wait for USB enumeration (`keyboard_is_connected()`)
3. Blink LED to indicate connection (`led_blink()`)
4. Apply initial delay if configured (cooperative wait with `keyboard_poll()`)
5. `engine_start()` if valid script exists
6. Main loop: `keyboard_poll()` + `engine_tick()`

**Dependencies:** `eeprom_storage.h`, `usb_keyboard.h`, `usb_rawhid.h`, `script_engine.h`, `timer.h`, `led.h`,
`usbdrv.h`

### 3. usb_core.c/h (USB Lifecycle)

**Purpose:** Encapsulates V-USB initialization and polling. Acts as the interface between the application layer and the
V-USB driver.

**Public API:**

```c
void usb_init(void);    /* Initialize V-USB (disconnect/connect sequence) */
void usb_poll(void);    /* Poll V-USB driver */
```

**Dependencies:** `usbdrv.h`, `avr/io.h`

### 4. usb_dispatcher.c/h (V-USB Dispatcher)

**Purpose:** Implements V-USB callbacks and routes requests to the appropriate handler based on current device mode.

**V-USB callbacks (called by V-USB driver):**

```c
usbMsgLen_t usbFunctionSetup(uint8_t data[8]);          /* Route HID class requests */
usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len); /* Route SET_REPORT data */
uchar usbFunctionRead(uchar *data, uchar len);          /* Route GET_REPORT data */
usbMsgLen_t usbFunctionDescriptor(struct usbRequest *request); /* Dynamic descriptors */
```

**Routing logic:**

- HID class requests -> `keyboard_handle_setup()` or `rawhid_handle_setup()`
- Write data -> `keyboard_handle_write()` or `rawhid_handle_write()`
- Read data -> `rawhid_handle_read()` (keyboard returns 0)
- Descriptors -> `usb_descriptors` module

**Dependencies:** `device_mode.h`, `usb_descriptors.h`, `usb_rawhid.h`, `usb_keyboard.h`

### 5. usb_descriptors.c/h (Dynamic Descriptors)

**Purpose:** Provides USB descriptors based on current device mode. All descriptors are stored in PROGMEM.

**Public API:**

```c
usbMsgLen_t descriptors_get_configuration(void);  /* Configuration descriptor */
usbMsgLen_t descriptors_get_hid(void);             /* HID descriptor */
usbMsgLen_t descriptors_get_hid_report(void);      /* HID report descriptor */
```

**Descriptor sets:**

| Mode        | Interface Class | Subclass    | Protocol        | Usage Page             | Report Descriptor |
|-------------|-----------------|-------------|-----------------|------------------------|-------------------|
| Programming | 0x03 (HID)      | 0x00 (None) | 0x00 (None)     | 0xFF00 (Vendor)        | 29 bytes          |
| Keyboard    | 0x03 (HID)      | 0x01 (Boot) | 0x01 (Keyboard) | 0x01 (Generic Desktop) | 63 bytes          |

**usbconfig.h integration:** Dynamic descriptors are enabled via:

```c
#define USB_CFG_DESCR_PROPS_CONFIGURATION   USB_PROP_IS_DYNAMIC
#define USB_CFG_DESCR_PROPS_HID             USB_PROP_IS_DYNAMIC
#define USB_CFG_DESCR_PROPS_HID_REPORT      USB_PROP_IS_DYNAMIC
```

**Dependencies:** `device_mode.h`

### 6. usb_rawhid.c/h (Programming Mode USB)

**Purpose:** Handles Raw HID USB communication. Receives SET_REPORT data from host, buffers it, dispatches complete
reports to `hid_protocol`, and returns responses via GET_REPORT.

**Public API:**

```c
void rawhid_init(void);                                              /* Reset state */
usbMsgLen_t rawhid_handle_setup(usbRequest_t *request);             /* HID class requests */
usbMsgLen_t rawhid_handle_write(uint8_t *data, uint8_t length);     /* Incoming data */
uint8_t rawhid_handle_read(uint8_t *data, uint8_t length);          /* Outgoing data */
bool rawhid_has_pending_response(void);                              /* Response ready? */
bool rawhid_should_exit(void);                                       /* CMD_EXIT received? */
bool rawhid_had_activity(void);                                      /* Any report received? */
```

**Dependencies:** `config.h`, `hid_protocol.h`

### 7. hid_protocol.c/h (Command Processing)

**Purpose:** Implements the HID report protocol for programming scripts via WebHID. See
`firmware/spec/hid-report-protocol.md` for full protocol specification.

**Commands:**

| Code | Command | Description                       | Stateful |
|------|---------|-----------------------------------|----------|
| 0x01 | WRITE   | Write bytes to script area        | No       |
| 0x02 | READ    | Read bytes from script area       | No       |
| 0x03 | APPEND  | Sequential write with running CRC | Yes      |
| 0x04 | RESET   | Reset offset and CRC              | Yes      |
| 0x05 | COMMIT  | Validate CRC and write header     | Yes      |
| 0x06 | STATUS  | Get device info and state         | No       |
| 0x07 | EXIT    | Transition to keyboard mode       | No       |

**Public API:**

```c
void protocol_init(void);                                          /* Reset state */
void protocol_process_report(const uint8_t *report, uint8_t length); /* Dispatch command */
const uint8_t* protocol_get_response(void);                        /* Get response */
uint8_t protocol_get_response_length(void);                        /* Response size */
bool protocol_exit_requested(void);                                /* CMD_EXIT flag */
```

**Internal state:**

- `current_offset` (uint16_t): Script-relative write offset, starts at 0
- `running_crc` (uint16_t): Accumulated CRC of appended bytes
- `exit_requested` (bool): Set by CMD_EXIT

**Important:**

- `WRITE` and `READ` commands use **absolute EEPROM addresses** (0x0000 - 0x01FF). The protocol handler validates that
  operations stay within valid ranges.
- `APPEND` uses an internal `current_offset` which is **script-relative**. The handler adds `STORAGE_SCRIPT_START` (8)
  before writing to storage.

**Dependencies:** `config.h`, `eeprom_storage.h`, `crc16.h`

### 8. usb_keyboard.c/h (Keyboard Mode USB)

**Purpose:** Handles Boot Protocol HID keyboard communication. Manages the 8-byte keyboard report buffer and USB
interrupt transfers.

**Constants:**

```c
#define KEYBOARD_REPORT_SIZE 8   /* Standard 8-byte boot protocol report */
#define KEYBOARD_MAX_KEYS    6   /* Maximum simultaneous keys (6KRO) */
```

**Public API:**

```c
/* Lifecycle */
void keyboard_init(void);                 /* Reset report buffer and state */

/* USB Maintenance */
void keyboard_poll(void);                 /* Call usbPoll() + handle idle reports */
bool keyboard_is_ready(void);             /* Can send a report? */
bool keyboard_is_connected(void);         /* Host has communicated? */

/* Report Sending */
bool keyboard_send_report(uint8_t modifiers, const uint8_t *keys, uint8_t key_count);
void keyboard_release_all(void);          /* Send empty report */

/* LED State */
uint8_t keyboard_get_led_state(void);     /* Caps/Num/Scroll Lock from host */

/* USB Handlers (called by usb_core.c) */
usbMsgLen_t keyboard_handle_setup(usbRequest_t *request);
usbMsgLen_t keyboard_handle_write(uint8_t *data, uint8_t length);
```

`keyboard_init()` only resets internal state (report buffer, idle rate, protocol version, LED state). USB
initialization is handled separately by `device_mode.c:init_usb()`.

**Dependencies:** V-USB driver (`usbdrv.h`)

### 9. script_engine.c/h (Bytecode Interpreter)

**Purpose:** Executes scripts stored in EEPROM. Reads bytecode opcodes and performs keyboard actions. Must be called
cooperatively from the main loop via `engine_tick()`.

**Opcodes:**

| Code | Opcode   | Arguments       | Description                     |
|------|----------|-----------------|---------------------------------|
| 0x00 | END      | -               | Stop script execution           |
| 0x01 | DELAY    | duration(2)     | Wait for N milliseconds         |
| 0x02 | KEY_DOWN | keycode(1)      | Press key                       |
| 0x03 | KEY_UP   | keycode(1)      | Release key                     |
| 0x04 | MOD      | modifier(1)     | Set modifier byte               |
| 0x05 | TAP      | keycode(1)      | Press + release key             |
| 0x06 | REPEAT   | count(1)+len(1) | Repeat next N bytes count times |
| 0x07 | COMBO    | mod(1)+key(1)   | Modifier + key combination      |
| 0x08 | STRING   | len(1)+chars(N) | Type ASCII string               |

**Types:**

```c
typedef enum {
    ENGINE_IDLE,      /* Not started */
    ENGINE_RUNNING,   /* Executing opcodes */
    ENGINE_DELAYING,  /* Waiting for delay to complete */
    ENGINE_FINISHED,  /* Script ended (OP_END) */
    ENGINE_ERROR      /* Invalid opcode */
} engine_state_t;
```

**Public API:**

```c
void engine_init(void);                /* Reset state to IDLE */
void engine_start(void);              /* Begin execution from offset 0 */
void engine_stop(void);               /* Stop and reset */
engine_state_t engine_tick(void);     /* Execute one step, return state */
engine_state_t engine_get_state(void);
bool engine_is_running(void);
```

**Dependencies:** `config.h`, `eeprom_storage.h`, `usb_keyboard.h`, `keycode.h`, `timer.h`

### 10. eeprom_storage.c/h (Script Storage)

**Purpose:** Manages EEPROM read/write operations with header validation and CRC integrity.

**EEPROM Layout:**

```
Offset  Size  Field
------  ----  -----
0x000   1     VERSION (0x1A = payload format version)
0x001   1     FLAGS (reserved, 0x00)
0x002   2     DELAY (initial delay × 100ms, little-endian)
0x004   2     LENGTH (script length in bytes, little-endian)
0x006   2     CRC16 (CRC of script data, little-endian)
0x008   504   Script bytecode
```

**Public API:**

```c
/* Lifecycle */
void storage_init(void);

/* Reading (Absolute EEPROM address) */
uint8_t storage_read_byte(uint16_t address);
void storage_read_bytes(uint16_t address, uint8_t *buffer, uint8_t length);

/* Script Metadata */
uint16_t storage_get_script_length(void);
uint16_t storage_get_initial_delay(void);      /* Returns delay in ms (stored value × 100) */

/* Writing (Absolute EEPROM address) */
void storage_write_byte(uint16_t address, uint8_t value);
void storage_write_bytes(uint16_t address, const uint8_t *data, uint8_t length);

/* Header Operations */
void storage_write_header(uint8_t version, uint8_t flags, uint16_t delay, uint16_t length, uint16_t crc);
void storage_invalidate_script(void);          /* Sets LENGTH to 0 */

/* Validation */
bool storage_has_valid_script(void);           /* VERSION == 0x1A AND LENGTH > 0 */
bool storage_verify_crc(uint16_t length, uint16_t expected_crc);

```

**Important:** `storage_read_byte` and `storage_write_byte` use **absolute EEPROM addresses**. Callers are responsible
for calculating correct addresses. The module prevents writes outside valid EEPROM range.

All writes use `eeprom_update_byte()` (only writes if value differs) to minimize EEPROM wear.

**Dependencies:** `config.h`, `crc16.h`

### 11. keycode.c/h (ASCII to HID Keycode)

**Purpose:** Converts ASCII characters (0x00-0x7F) to USB HID keycodes with modifier information. Uses US keyboard
layout. Provides lookup via a PROGMEM table.

**Types:**

```c
typedef struct {
    uint8_t keycode;    /* USB HID keycode */
    uint8_t modifiers;  /* Modifier mask */
} keycode_result_t;
```

**Public API:**

```c
keycode_result_t keycode_from_ascii(char c);
```

**Constants:** Defines modifier masks (`MOD_CTRL`, `MOD_SHIFT`, `MOD_ALT`, `MOD_GUI` and left/right variants), special
keys (`KEY_ENTER`, `KEY_TAB`, `KEY_BACKSPACE`, etc.), function keys (`KEY_F1`-`KEY_F12`), and navigation keys
(`KEY_ARROW_*`, `KEY_HOME`, `KEY_END`, etc.).

**Dependencies:** None (standalone module)

### 12. timer.c/h (Hardware Timer)

**Purpose:** Provides millisecond-resolution timing using Timer1 on the ATtiny85. Non-blocking design — callers must
poll `usbPoll()` or `keyboard_poll()` during waits.

**Public API:**

```c
void timer_init(void);                                /* Configure Timer1 */
uint16_t timer_millis(void);                          /* Current time (wraps at 65535) */
bool timer_elapsed(uint16_t start, uint16_t duration); /* Check if duration has passed */
```

`timer_elapsed()` handles 16-bit wraparound correctly.

**Dependencies:** None (standalone module, uses AVR Timer1 hardware)

### 13. crc16.c/h (CRC Calculation)

**Purpose:** CRC-16-CCITT calculation for script integrity verification.

**Algorithm:**

- Polynomial: 0x1021 (CRC-16-CCITT)
- Initial value: 0xFFFF
- Process: MSB first
- No final XOR

**Public API:**

```c
uint16_t crc16_init(void);                           /* Returns CRC16_INIT (0xFFFF) */
uint16_t crc16_update(uint16_t crc, uint8_t byte);   /* Update CRC with one byte */
uint16_t crc16_finalize(uint16_t crc);               /* Return final CRC value */
```

**Dependencies:** `config.h` (for `CRC16_INIT`, `CRC16_POLY`)

### 14. oscillator.c/h (Oscillator Calibration)

**Purpose:** Calibrates the ATtiny85 internal RC oscillator for stable USB timing. Called automatically by V-USB during
USB enumeration via `USB_RESET_HOOK`.

**Public API:**

```c
void calibrate_oscillator(void);
```

**usbconfig.h integration:**

```c
extern void calibrate_oscillator(void);
#define USB_RESET_HOOK(resetStarts) if(!resetStarts){cli(); calibrate_oscillator(); sei();}
```

**Dependencies:** V-USB driver (`usbMeasureFrameLength()`)

### 15. led.c/h (LED Control)

**Purpose:** Controls the onboard LED on PB1 (Digispark LED). Used for mode indication: LED on = programming mode,
LED off = keyboard mode.

**Constants:**

```c
#define LED_PIN PB1
```

**Public API:**

```c
void led_init(void);      /* Configure PB1 as output */
void led_on(void);
void led_off(void);
void led_toggle(void);
bool led_is_on(void);

/* Status Indication */
void led_blink(uint8_t count, uint16_t on_ms, uint16_t off_ms, void (*idle_callback)(void));
```

**Dependencies:** None (standalone module, uses AVR GPIO)

## usbconfig.h

V-USB configuration file. Key settings:

| Setting                             | Value                    | Purpose                          |
|-------------------------------------|--------------------------|----------------------------------|
| `USB_CFG_IOPORTNAME`                | B                        | ATtiny85 Port B                  |
| `USB_CFG_DMINUS_BIT`                | 3                        | D- on PB3                        |
| `USB_CFG_DPLUS_BIT`                 | 4                        | D+ on PB4 (with INT0)            |
| `USB_CFG_HAVE_INTRIN_ENDPOINT`      | 1                        | Enable interrupt endpoint        |
| `USB_CFG_IMPLEMENT_FN_WRITE`        | 1                        | Enable `usbFunctionWrite`        |
| `USB_CFG_IMPLEMENT_FN_READ`         | 1                        | Enable `usbFunctionRead`         |
| `USB_CFG_DESCR_PROPS_CONFIGURATION` | `USB_PROP_IS_DYNAMIC`    | Dynamic configuration descriptor |
| `USB_CFG_DESCR_PROPS_HID`           | `USB_PROP_IS_DYNAMIC`    | Dynamic HID descriptor           |
| `USB_CFG_DESCR_PROPS_HID_REPORT`    | `USB_PROP_IS_DYNAMIC`    | Dynamic HID report descriptor    |
| `USB_RESET_HOOK`                    | `calibrate_oscillator()` | RC oscillator calibration        |
| USB Device Version                  | 0x0001                   | v1.00                            |

---

## Memory Budget (Estimated)

| Component        | Flash (bytes) | RAM (bytes) |
|------------------|---------------|-------------|
| V-USB driver     | ~1,400        | 50-80       |
| Keyboard mode    | ~800          | 50          |
| Programming mode | ~600          | 64          |
| Protocol handler | ~400          | 40          |
| Descriptors      | ~200          | 0           |
| Storage          | ~300          | 10          |
| Script engine    | ~600          | 30          |
| Timer            | ~100          | 4           |
| CRC16            | ~50           | 0           |
| Other utilities  | ~100          | 10          |
| **Total (est.)** | **~4,550**    | **~300**    |
| **Available**    | **~6,000**    | **512**     |

---

## Protocol Reference

See `firmware/spec/hid-report-protocol.md` for complete HID report protocol specification.

| Command | Code | Description                       |
|---------|------|-----------------------------------|
| WRITE   | 0x01 | Write bytes to script area        |
| READ    | 0x02 | Read bytes from script area       |
| APPEND  | 0x03 | Sequential write with running CRC |
| RESET   | 0x04 | Reset state variables             |
| COMMIT  | 0x05 | Validate CRC and write header     |
| STATUS  | 0x06 | Get device info and state         |
| EXIT    | 0x07 | Transition to keyboard mode       |

---

## Important Notes

1. **No blocking delays in main loops** — All delays use cooperative polling with `keyboard_poll()` or `usbPoll()`
2. **EEPROM writes use update semantics** — `eeprom_update_byte()` only writes if value differs, extending EEPROM life
3. **Absolute vs Relative Addressing** — `eeprom_storage` uses absolute EEPROM addresses. The protocol's `WRITE` and
   `READ` commands expose absolute addressing to the host. `APPEND` operations are script-relative and offset by
   `STORAGE_SCRIPT_START` (8) by the protocol handler.
4. **Mode detection via MCUSR/GPIOR0** — Watchdog reset flag (WDRF) determines boot mode, no EEPROM flag needed
5. **Watchdog reset for mode transition** — Simpler and faster than USB re-enumeration with V-USB
6. **Script validation** — A script is valid when `VERSION == STORAGE_PAYLOAD_VERSION` (0x1A) AND `LENGTH > 0`
7. **Script invalidation on COMMIT failure** — Sets LENGTH to 0 at `HEADER_OFFSET_LENGTH`
8. **All shared constants in config.h** — Derived values are calculated, not hardcoded
9. **Dynamic USB descriptors** — `usbFunctionDescriptor()` serves different descriptors based on mode, enabled via
   `USB_PROP_IS_DYNAMIC` in `usbconfig.h`
10. **Code style** — See `AGENTS.md` for coding conventions (snake_case, 76-char dash separators, etc.)
