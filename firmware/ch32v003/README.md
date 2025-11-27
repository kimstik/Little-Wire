# Little-Wire CH32V003 Port

```
@author kimstik
assisted by intelligence
```

## Overview

This is a port of Little-Wire firmware to the CH32V003 RISC-V microcontroller, using the [rv003usb](https://github.com/cnlohr/rv003usb) software USB implementation.

## Features

- Full compatibility with original Little-Wire protocol (v1.3)
- Works with existing Little-Wire host library
- Same VID/PID as original Little-Wire
- Firmware version: 0x20 (2.0)

## Supported Functions

| Feature | Status |
|---------|--------|
| GPIO (digital I/O) | ✅ |
| ADC (analog input) | ✅ |
| Hardware PWM | ✅ |
| Software PWM | ✅ |
| SPI (hardware + debug) | ✅ |
| I2C (bitbang) | ✅ |
| OneWire | ✅ |
| WS2812 LEDs | ✅ |

## Pin Mapping

| Little-Wire Pin | CH32V003 Pin | Functions |
|-----------------|--------------|-----------|
| PIN1 | PC1 | GPIO, PWM (TIM1_CH1), I2C SDA |
| PIN2 | PC2 | GPIO, PWM (TIM1_CH2), I2C SCL |
| PIN3 | PC3 | GPIO |
| PIN4 | PC4 | GPIO, OneWire |
| USB D+ | PD3 | USB |
| USB D- | PD4 | USB |
| USB DPU | PD5 | USB Pull-up |

### SPI Pins

| Function | CH32V003 Pin |
|----------|--------------|
| SCK | PC5 |
| MOSI | PC6 |
| MISO | PC7 |

### ADC Channels

| Channel | CH32V003 Pin |
|---------|--------------|
| 0 | PA2 |
| 1 | PA1 |
| 2 (temp) | Internal |

## Hardware Requirements

- CH32V003 microcontroller (TSSOP20 recommended for full functionality)
- 1.5kΩ resistor on USB D- (or use DPU pin control)
- USB Type-C or Micro-B connector
- 3.3V power supply (or from USB with regulator)

### Minimal Circuit

```
                CH32V003 (TSSOP20)
            ┌──────────────────────┐
    USB D-  │ PD4             VDD  │──── 3.3V
            │                      │
    USB DPU │ PD5             VSS  │──── GND
            │                      │
            │ PD6             PC7  │──── SPI MISO
            │                      │
    NRST    │ PD7             PC6  │──── SPI MOSI
            │                      │
    ADC1    │ PA1             PC5  │──── SPI SCK
            │                      │
    ADC0    │ PA2             PC4  │──── PIN4 / OneWire
            │                      │
    GND     │ VSS             PC3  │──── PIN3
            │                      │
            │ PD0             PC2  │──── PIN2 / I2C SCL
            │                      │
    SWIO    │ PD1             PC1  │──── PIN1 / I2C SDA
            │                      │
            │ PC0             PD2  │
            │                      │
            │ PD3                  │
    USB D+  └──────────────────────┘

    USB Connection:
    - D+ to PD3 via 33Ω resistor (optional)
    - D- to PD4 via 33Ω resistor (optional)
    - 1.5kΩ pull-up from D- to 3.3V (or controlled via PD5)
```

## Building

### Prerequisites

1. Install RISC-V GCC toolchain
2. Install [minichlink](https://github.com/cnlohr/ch32v003fun/tree/master/minichlink) for flashing

### Build Commands

```bash
# Initialize submodules (first time only)
git submodule update --init --recursive

# Build firmware
make

# Build and flash
make flash

# Clean build
make clean
```

## Usage

Once flashed, the device will appear as a USB device with:
- Vendor ID: 0x1781
- Product ID: 0x0c9f
- Device Name: "Little-Wire CH32V003"

Use the standard Little-Wire host library to communicate with the device.

```c
#include "littleWire.h"

int main() {
    littleWire* lw = littleWire_connect();

    // Check firmware version
    printf("Version: 0x%02X\n", readFirmwareVersion(lw));
    // Should print: Version: 0x20

    // Use like normal Little-Wire
    pinMode(lw, PIN1, OUTPUT);
    digitalWrite(lw, PIN1, HIGH);

    return 0;
}
```

## Differences from ATtiny85 Version

| Aspect | ATtiny85 | CH32V003 |
|--------|----------|----------|
| Clock | 16.5 MHz | 48 MHz |
| Flash | 8 KB | 16 KB |
| RAM | 512 B | 2 KB |
| GPIO | 6 pins | 18 pins |
| Cost | ~$1.50 | ~$0.10 |
| USB | V-USB | rv003usb |
| SPI | USI/bitbang | Hardware |
| I2C | Bitbang | Bitbang* |

*Hardware I2C available but bitbang used for compatibility

## License

GPL v2 (same as original Little-Wire)

## Credits

- Original Little-Wire: ihsan Kehribar, chris chung
- rv003usb: CNLohr
- ch32v003fun: CNLohr
- CH32V003 port: kimstik, assisted by intelligence
