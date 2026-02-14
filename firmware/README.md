# TinyKB Firmware

USB keyboard firmware for ATtiny85/Digispark that executes macro scripts stored in EEPROM.

## Documentation

For detailed technical documentation, please refer to the `docs/` directory:

- [**Architecture**](docs/architecture.md): System overview, design decisions, and module structure.
- [**Script Bytecode**](docs/script-bytecode-specification.md): Complete list of opcodes and script format.
- [**HID Protocol**](docs/hid-programming-specification.md): USB communication protocol for programming.
- [**Hardware Reference**](docs/hardware-reference.md):  ATtiny85/Digispark pinout and fuse settings.
- [**HID Keycodes**](docs/hid-keycodes-reference.md): USB HID usage tables.

## Quick Start

### Build

```bash
bash build.sh
```

### Flash

```bash
micronucleus --run build/tinykb.hex
```

### Development Tools

- [AVR-GCC](https://gcc.gnu.org/wiki/avr-gcc)
- [AVR-Libc](https://github.com/avrdudes/avr-libc/)
- [Micronucleus](https://github.com/micronucleus/micronucleus)
