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
- WSL or Linux environment

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

- Digispark/ATtiny85 with Micronucleus bootloader
- micronucleus.exe flasher tool

### Flash Commands

**Windows:**

```cmd
micronucleus.exe --run build/tinykb.hex
```

### Flashing Process

1. Run flash command
2. Connect Digispark within 60 seconds
3. Firmware uploads automatically
4. Device reconnects with new firmware

## Hardware Configuration

- **Target**: ATtiny85 @ 16.5MHz (internal RC oscillator)
- **USB Pins**: `D-` on PB3, `D+` on PB4 (INT0)
- **Bootloader**: Micronucleus v2.6 (occupies ~2KB flash)
- **Available Flash**: ~6KB for application code

### Memory Layout

**Flash Usage:**

- Application code: ~3.5KB
- Bootloader: ~2KB
- Available: ~2.5KB for expansion

**EEPROM Usage:**

- Script storage: 0-511 bytes
- Header format: magic, version, flags, CRC16

### USB Configuration

- **Vendor ID**: 0x16c0 (voti.nl)
- **Product ID**: 0x27db
- **Device Class**: HID Keyboard
- **Power**: Bus-powered, 100mA max
- **Speed**: Low-speed USB 1.1

## References

### Development Tools

- [AVR-GCC](https://gcc.gnu.org/wiki/avr-gcc): Cross-compiler for AVR microcontrollers
- [AVR-Libc](https://github.com/avrdudes/avr-libc/): Standard library for AVR
- [AVRDUDE](https://github.com/avrdudes/avrdude): AVR programming utility
- [Micronucleus](https://github.com/micronucleus/micronucleus): USB bootloader for ATtiny85

### Hardware Documentation

- [ATtiny85 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf):
  Official microcontroller datasheet

### USB Implementation

- [V-USB](https://www.obdev.at/products/vusb/index.html): Software-only USB driver for AVR
- [USB HID Usage Tables](https://usb.org/document-library/hid-usage-tables-17): Official USB HID specification

### Reference Projects

- [Adafruit TrinketKeyboard](https://github.com/adafruit/Adafruit-Trinket-USB): Similar USB keyboard implementation
- [DigiKeyboard Library](https://github.com/digistump/DigistumpArduino): Arduino library for USB keyboard
