#!/bin/bash
# TinyKB Firmware Build Script

set -e

echo "Building TinyKB firmware..."

# Build configuration
MCU=attiny85
F_CPU=16500000UL
BUILD_DIR=build
SRC_DIR=src
USB_LIB_DIR=lib/usbdrv

# Compiler flags and settings
CFLAGS="-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -std=c99 -ffunction-sections -fdata-sections"
INCLUDES="-I${SRC_DIR} -I${USB_LIB_DIR}"
LDFLAGS="-mmcu=${MCU} -Wl,--gc-sections"

# Create build directory
mkdir -p ${BUILD_DIR}

echo "Compiling source files..."

# Phase 1: Infrastructure modules
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/timer.c -o ${BUILD_DIR}/timer.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/keycode.c -o ${BUILD_DIR}/keycode.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/crc16.c -o ${BUILD_DIR}/crc16.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/led.c -o ${BUILD_DIR}/led.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/oscillator.c -o ${BUILD_DIR}/oscillator.o

# Phase 2: Mode management
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/eeprom_storage.c -o ${BUILD_DIR}/eeprom_storage.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/device_mode.c -o ${BUILD_DIR}/device_mode.o

# Phase 3: USB layer
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/usb_descriptors.c -o ${BUILD_DIR}/usb_descriptors.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/usb_core.c -o ${BUILD_DIR}/usb_core.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/usb_dispatcher.c -o ${BUILD_DIR}/usb_dispatcher.o

# Phase 4: Programming mode
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/hid_protocol.c -o ${BUILD_DIR}/hid_protocol.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/usb_rawhid.c -o ${BUILD_DIR}/usb_rawhid.o

# Phase 5: Keyboard mode
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/usb_keyboard.c -o ${BUILD_DIR}/usb_keyboard.o
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/script_engine.c -o ${BUILD_DIR}/script_engine.o

# Phase 6: Integration
avr-gcc $CFLAGS $INCLUDES -c ${SRC_DIR}/main.c -o ${BUILD_DIR}/main.o

echo "Compiling V-USB library..."

# Compile V-USB components
avr-gcc $CFLAGS $INCLUDES -c ${USB_LIB_DIR}/usbdrv.c -o ${BUILD_DIR}/usbdrv.o
avr-gcc $CFLAGS $INCLUDES -x assembler-with-cpp -c ${USB_LIB_DIR}/usbdrvasm.S -o ${BUILD_DIR}/usbdrvasm.o

echo "Linking..."

# Link all object files
avr-gcc $LDFLAGS -o ${BUILD_DIR}/tinykb.elf ${BUILD_DIR}/*.o

echo "Generating Intel HEX file..."

# Generate flashable HEX file
avr-objcopy -O ihex -R .eeprom ${BUILD_DIR}/tinykb.elf ${BUILD_DIR}/tinykb.hex

echo "Firmware size:"
avr-size --mcu=${MCU} -C ${BUILD_DIR}/tinykb.elf

echo ""
echo "Build completed -> ${BUILD_DIR}/tinykb.hex"
echo ""
echo "Flash commands:"
echo "  Windows: micronucleus.exe --run ${BUILD_DIR}/tinykb.hex"
echo "  WSL:     powershell.exe -Command \"micronucleus.exe --run \$(wslpath -w \$(pwd)/${BUILD_DIR}/tinykb.hex)\""
