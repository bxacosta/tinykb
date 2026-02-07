# Hardware Reference

This document provides a comprehensive reference for the Atmel ATtiny85/Digispark hardware, an 8-bit AVR processor with
RISC architecture.

--- 

## ATtiny85 Overview

| Specification     | Value                                      |
|-------------------|--------------------------------------------|
| Architecture      | AVR 8-bit RISC                             |
| Factory Speed     | 1 MHz (default internal RC oscillator)     |
| Digispark Speed   | 16.5 MHz (PLL enabled, calibrated for USB) |
| Operating Voltage | 5V (via USB)                               |
| GPIO Pins         | 6 (PB0-PB5)                                |
| ADC Channels      | 4 (10-bit resolution)                      |
| Interfaces        | USI (I2C / SPI)                            |

### Digispark Pinout

```
   Digispark ATtiny85
 _______________________
|                       |
|                       |--- PB5
|                       |--- PB4
|                       |--- PB3
|                       |--- PB2
|                       |--- PB1
|  [5V]   [GND]   [VIN] |--- PB0
|___|_______|_______|___|
    |       |       |
   5V      GND    6-16V
```

### Pin Functions

| Port | Chip Pin | GPIO | ADC  | PWM          | I2C | SPI  | PCINT  | Notes                     |
|------|----------|------|------|--------------|-----|------|--------|---------------------------|
| PB0  | 5        | Y    | -    | OC0A / ~OC1A | SDA | MOSI | PCINT0 | AREF (Analog Reference)   |
| PB1  | 6        | Y    | -    | OC0B / OC1A  | -   | MISO | PCINT1 | Onboard LED (Model A)     |
| PB2  | 7        | Y    | ADC1 | -            | SCL | SCK  | PCINT2 | INT0 (External Interrupt) |
| PB3  | 2        | Y    | ADC3 | ~OC1B        | -   | -    | PCINT3 | USB D-, 1.5k pull-up      |
| PB4  | 3        | Y    | ADC2 | OC1B         | -   | -    | PCINT4 | USB D+                    |
| PB5  | 1        | *    | ADC0 | -            | -   | -    | PCINT5 | Reset pin                 |

- PB5 is the Reset pin by default. To use it as GPIO, the `RSTDISBL` fuse must be programmed. This disables the
  reset function and requires high-voltage programming (HVSP) to reprogram the chip. Not recommended
- PB3 and PB4 are **not available as regular GPIO** when USB is active

### Port B Registers

The ATtiny85 GPIO is controlled via three 8-bit registers. Each bit corresponds to a pin (bit 0 = PB0, bit 5 = PB5).

| Register | Address | R/W | Description                                 |
|----------|---------|-----|---------------------------------------------|
| DDRB     | 0x17    | R/W | Data Direction Register (0=input, 1=output) |
| PORTB    | 0x18    | R/W | Data Register (output value / pull-up)      |
| PINB     | 0x16    | R   | Pin Input Register (read current state)     |

**Register Bit Layout:**

```
Bit:          7       6       5       4       3       2       1       0
          +-------+-------+-------+-------+-------+-------+-------+-------+
DDRB:     |   -   |   -   |  DDB5 |  DDB4 |  DDB3 |  DDB2 |  DDB1 |  DDB0 |
          +-------+-------+-------+-------+-------+-------+-------+-------+
PORTB:    |   -   |   -   |  PB5  |  PB4  |  PB3  |  PB2  |  PB1  |  PB0  |
          +-------+-------+-------+-------+-------+-------+-------+-------+
PINB:     |   -   |   -   | PINB5 | PINB4 | PINB3 | PINB2 | PINB1 | PINB0 |
          +-------+-------+-------+-------+-------+-------+-------+-------+
```

**Configuration Logic:**

| DDRB | PORTB | Mode                        |
|------|-------|-----------------------------|
| 0    | 0     | Input (Hi-Z, floating)      |
| 0    | 1     | Input with internal pull-up |
| 1    | 0     | Output LOW                  |
| 1    | 1     | Output HIGH                 |

**Usage Examples:**

```c
// Configure PB1 as output (for LED)
DDRB |= (1 << PB1);

// Set PB1 HIGH
PORTB |= (1 << PB1);

// Set PB1 LOW
PORTB &= ~(1 << PB1);

// Toggle PB1
PORTB ^= (1 << PB1);

// Configure PB2 as input with pull-up
DDRB &= ~(1 << PB2);
PORTB |= (1 << PB2);

// Read PB2 state
uint8_t state = (PINB & (1 << PB2)) ? 1 : 0;

// Clock Configuration for USB
PLLCSR = (1<<PLLE);  // Enable PLL
while (!(PLLCSR & (1<<PLOCK))); // Wait for PLL lock
PLLCSR |= (1<<PCKE); // Use PLL as clock source
```

---

## Memory Architecture

### Flash (Program Memory)

| Region        | Size        | Notes                           |
|---------------|-------------|---------------------------------|
| Total         | 8 KB        | Program storage                 |
| Bootloader    | 1510 B      | Micronucleus v2.6 (t85_default) |
| **Available** | **~6.5 KB** | For application code            |

### SRAM (Data Memory)

| Total | Notes                                   |
|-------|-----------------------------------------|
| 512 B | Runtime variables, stack, V-USB buffers |

### EEPROM (Non-Volatile Storage)

| Total | Write Cycles  | Notes                   |
|-------|---------------|-------------------------|
| 512 B | ~100,000/cell | Persistent data storage |

---

## Bootloader (Micronucleus)

The Digispark comes pre-loaded with the [Micronucleus Bootloader](https://github.com/micronucleus/micronucleus), which
allows firmware updates over USB without an external programmer.

| Property | Value                                   |
|----------|-----------------------------------------|
| Version  | v2.6                                    |
| Size     | 1510 bytes (t85_default configuration)  |
| Protocol | USB (no external programmer required)   |
| Timeout  | 5 seconds (default `ENTRY_ALWAYS` mode) |

### Entry Modes

| Mode             | Behavior                                  |
|------------------|-------------------------------------------|
| `ENTRY_ALWAYS`   | Bootloader runs on every reset (5s wait)  |
| `ENTRY_POWER_ON` | Bootloader only on power-on, skips on WDT |

### Flash Tool

```bash
micronucleus --run firmware.hex
```

---

## USB Implementation (V-USB)

The Digispark implements USB via software using the [V-USB](https://www.obdev.at/products/vusb/) library.
Since the ATtiny85 has no hardware USB peripheral, V-USB bit-bangs the USB protocol in software.

### Characteristics

| Property       | Value                        |
|----------------|------------------------------|
| Speed          | Low-Speed USB 1.1 (1.5 Mbps) |
| Implementation | Software (bit-banging)       |
| Max Packet     | 8 bytes (Low-Speed limit)    |

### Pin Configuration

```c
D- (DMINUS): PB3  // Has 1.5k pull-up (required for Low-Speed)
D+ (DPLUS):  PB4  // Used for edge detection
```

**Important:** The 1.5k pull-up resistor on PB3 is hardwired on the Digispark board. USB Low-Speed specification
requires this pull-up on `D-` to identify the device as Low-Speed. The firmware must configure `D-` on PB3 to match
this hardware design.

### Timing Requirements

The internal PLL must be configured to 16.5 MHz (±1%) for reliable USB operation. V-USB handles this calibration
automatically during USB reset via `USB_RESET_HOOK`.

---

## Constraints & Limitations

### USB Low-Speed Limitations

- Maximum interrupt transfer: **8 bytes per packet**
- Larger reports are fragmented by V-USB automatically
- No isochronous or bulk transfers

### Memory Constraints

| Resource | Limit   | Practical Impact                 |
|----------|---------|----------------------------------|
| Flash    | ~6.5 KB | Limits firmware complexity       |
| RAM      | ~450 B  | Limits buffer sizes, stack depth |
| EEPROM   | 512 B   | Limits script storage size       |

### Timing Sensitivity

USB timing is critical. Long interrupt-disabled sections (>10us) may cause USB errors. The firmware must call
`usbPoll()` frequently.

---

## References

- [ATtiny85 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf)
- [V-USB Library](https://www.obdev.at/products/vusb/index.html)
- [Micronucleus Bootloader](https://github.com/micronucleus/micronucleus)
- [Registers Reference](https://evan.widloski.com/notes/attiny/)
- [ATTinyCore Documentation](https://github.com/SpenceKonde/ATTinyCore/blob/OldMaster---DO-NOT-SUBMIT-PRs-here-Use-2.0.0-dev/avr/extras/ATtiny_x5.md)
