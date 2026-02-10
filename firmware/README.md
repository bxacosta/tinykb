# TinyKB Firmware

USB keyboard firmware for ATtiny85/Digispark that executes macro scripts stored in EEPROM.

## Architecture

```
firmware/
├── src/
│   ├── main.c              # Entry point, program window, main loop
│   ├── usb_core.c/h        # USB request dispatcher
│   ├── usb_keyboard.c/h    # USB HID keyboard interface
│   ├── usb_vendor.c/h      # USB vendor requests for programming
│   ├── script_engine.c/h   # Bytecode interpreter
│   ├── timer.c/h           # Millisecond timing
│   ├── eeprom_storage.c/h  # Script storage with CRC16 validation
│   ├── crc16.c/h           # CRC16-CCITT utility
│   ├── keycode.c/h         # ASCII to HID keycode conversion
│   └── usbconfig.h         # V-USB configuration
│
└── lib/usbdrv/             # V-USB library
```

### Core Modules

- **USB Core**: Dispatcher that routes USB requests to keyboard (HID) or vendor handlers.
- **USB Keyboard**: HID keyboard using V-USB. Includes oscillator calibration on USB reset.
- **USB Vendor**: Vendor requests for script programming via WebUSB.
- **Script Engine**: Bytecode interpreter. Supports keys, delays, modifiers, and strings.
- **Timer**: Millisecond-precision timing using Timer1 with interrupts.
- **EEPROM Storage**: Script storage with header validation and CRC16 integrity.
- **CRC16**: Shared CRC16-CCITT used by storage and vendor modules.
- **Keycode**: ASCII to USB HID keycode translation.

## Building

### Prerequisites

- AVR toolchain (avr-gcc, avr-objcopy, avr-size)

### Compilation

```bash
cd firmware
bash build.sh
```

The build process:

1. Compiles all source modules
2. Links with V-USB library
3. Generates `build/tinykb.hex` ready for flashing

## Flashing

### Requirements

- Micronucleus command line tool

### Flash Commands

```bash
micronucleus --run build/tinykb.hex
```

### Flashing Process

1. Run flash command
2. Connect Digispark within 60 seconds
3. Firmware uploads automatically

## References

- [ATtiny85/Digispark hardware hardware reference](./docs/hardware-reference.md)
- [Script Bytecode Specification](./docs/script-bytecode-specification.md)
- [HID Programming Specification](./docs/hid-programming-specification.md)
- [USB HID Keyboard Keycodes Reference](docs/hid-keycodes-reference.md)

### Development Tools

- [AVR-GCC](https://gcc.gnu.org/wiki/avr-gcc): Cross-compiler for AVR microcontrollers
- [AVR-Libc](https://github.com/avrdudes/avr-libc/): Standard library for AVR
- [AVRDUDE](https://github.com/avrdudes/avrdude): AVR programming utility
- [Micronucleus](https://github.com/micronucleus/micronucleus): USB bootloader for ATtiny85

### Reference Projects

- [Adafruit TrinketKeyboard](https://github.com/adafruit/Adafruit-Trinket-USB): Similar USB keyboard implementation
- [DigiKeyboard Library](https://github.com/digistump/DigistumpArduino): Arduino library for USB keyboard
