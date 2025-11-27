# Little-Wire CH32V003 Port - Deep Project Review

```
@author kimstik
assisted by intelligence
```

## Executive Summary

This document provides a comprehensive analysis of integrating CH32V003 USB support into the Little-Wire project using rv003usb as a git submodule.

---

## 1. Current Little-Wire Architecture

### 1.1 Project Structure

```
Little-Wire/
├── firmware/                    # ATtiny85 firmware
│   ├── main.c                  # Core firmware (1,456 lines)
│   ├── digital.h               # GPIO macros
│   ├── usbconfig.h             # V-USB configuration
│   ├── Makefile                # AVR build system
│   └── usbdrv/                 # V-USB driver (obdev.at)
│       ├── usbdrv.c/h          # USB core (~2,000 lines)
│       └── usbdrvasm.S         # Assembly USB handling
│
├── software/                   # Host-side library
│   ├── library/                # Cross-platform C API
│   │   ├── littleWire.h/c      # Main API (568+570 lines)
│   │   ├── littleWire_servo.h/c
│   │   ├── littleWire_util.h/c
│   │   └── opendevice.h/c      # USB discovery
│   ├── examples/               # 13 demo programs
│   └── Makefile
│
└── hardware/                   # Eagle PCB files
```

### 1.2 Firmware Protocol (35 USB Commands)

| Category | Commands | Request IDs |
|----------|----------|-------------|
| GPIO | PIN_SET_INPUT, OUTPUT, HIGH, LOW, READ | 13-14, 18-20 |
| ADC | READ_ADC, INIT_ADC | 15, 35 |
| PWM | SETUP, UPDATE_COMPARE, PRESCALE, STOP | 16-17, 22, 32 |
| SPI | SETUP, ADD_BUFFER, SEND_BUFFER, SINGLE, DEBUG | 21, 23, 28-29, 31, 33 |
| I2C | SETUP, BEGIN_TX, ADD_BUFFER, SEND, REQUEST, UPDATE_DELAY | 24-27, 30, 49 |
| OneWire | RESET, SEND_BYTE, READ_BYTE, READ_BIT, WRITE_BIT | 41-43, 50-51 |
| WS2812 | WS2812_WRITE | 54 |
| System | VERSION_QUERY, CHANGE_SERIAL | 34, 55 |

### 1.3 Current USB Implementation (V-USB)

- **MCU**: ATtiny85 @ 16.5MHz
- **USB Type**: Low-speed (1.5 Mbps)
- **VID/PID**: 0x1781:0x0c9f
- **Endpoints**: Control EP0 + Interrupt EP1
- **Driver**: Software USB (bitbang, ~2000 lines assembly)
- **Calibration**: RC oscillator via USB frames

### 1.4 Pin Mapping (ATtiny85)

| Pin | Function | Little-Wire Usage |
|-----|----------|-------------------|
| PB0 | GPIO/PWM0 | PIN4, I2C SDA |
| PB1 | GPIO/PWM1 | PIN1 |
| PB2 | GPIO/SCK | PIN2, I2C SCL, ADC1 |
| PB3 | USB D- | Reserved |
| PB4 | USB D+ | Reserved |
| PB5 | RESET | PIN3, ADC0 |

---

## 2. RV003USB Analysis

### 2.1 Project Overview

- **Repository**: https://github.com/cnlohr/rv003usb
- **MCU**: CH32V003 (RISC-V, ~$0.10)
- **USB Type**: Low-speed (software implementation)
- **Dependency**: ch32v003fun (minimal SDK, included as submodule)

### 2.2 Core Files (3 files only!)

```
rv003usb/
├── rv003usb.S    # Assembly USB interrupt handler
├── rv003usb.c    # C USB protocol stack (~250 lines)
└── rv003usb.h    # API headers and structures
```

### 2.3 Key Structures

```c
// Endpoint configuration
struct usb_endpoint {
    uint8_t count;          // ACK counter
    uint8_t toggle_in;      // DATA0/1 toggle
    uint8_t toggle_out;
    uint8_t custom;         // Custom handler flag
    uint8_t max_len;        // Max packet size
    void* opaque;           // User pointer
};

// USB Request Block
struct usb_urb {
    uint16_t wRequestTypeLSBRequestMSB;
    uint16_t lValueLSBIndexMSB;
    uint16_t wLength;
} __attribute__((packed));
```

### 2.4 Configuration Model

Each application provides `usb_config.h`:

```c
#define ENDPOINTS             2
#define USB_PORT              D
#define USB_PIN_DP            3
#define USB_PIN_DM            4
#define USB_PIN_DPU           5    // Pull-up control

// Feature flags
#define RV003USB_HANDLE_IN_REQUEST    1
#define RV003USB_OTHER_CONTROL        0
#define RV003USB_HANDLE_USER_DATA     0
```

### 2.5 API Functions

| Function | Description |
|----------|-------------|
| `usb_setup()` | Initialize USB stack |
| `usb_send_data(data, len, poly, token)` | Send data packet |
| `usb_send_empty(token)` | Send empty packet |
| `usb_handle_user_in_request()` | User callback for IN |
| `usb_handle_other_control_message()` | Custom control handler |
| `usb_handle_user_data()` | Non-control data handler |

### 2.6 Constraints

- **GPIO Pins**: Only GPIO 0-4 usable for D+/D- (instruction encoding limit)
- **Interrupt Priority**: USB interrupt MUST be highest, non-preemptible
- **Critical Sections**: Keep under 40 CPU cycles
- **Flash Size**: Basic HID ~2KB

---

## 3. Comparison Matrix

| Aspect | Little-Wire (ATtiny85) | RV003USB (CH32V003) |
|--------|------------------------|---------------------|
| **Architecture** | AVR 8-bit | RISC-V 32-bit |
| **Clock** | 16.5 MHz | 48 MHz |
| **Flash** | 8 KB | 16 KB |
| **RAM** | 512 B | 2 KB |
| **GPIO** | 6 pins | 18 pins |
| **Cost** | ~$1.50 | ~$0.10 |
| **USB Driver** | V-USB (~2000 LOC) | rv003usb (~250 LOC + ASM) |
| **ADC** | 10-bit, 4 ch | 10-bit, 8 ch |
| **PWM** | 2 ch (Timer0) | 4 ch (TIM1) |
| **SPI** | USI-based | Hardware SPI |
| **I2C** | Bitbang | Hardware I2C |

---

## 4. Integration Approach

### 4.1 Git Submodule Strategy

```
Little-Wire/
├── firmware/
│   ├── attiny85/           # Existing ATtiny85 code (renamed)
│   └── ch32v003/           # New CH32V003 port
│       ├── rv003usb/       # SUBMODULE: cnlohr/rv003usb
│       ├── main.c          # CH32V003 Little-Wire firmware
│       ├── usb_config.h    # USB configuration
│       ├── digital.h       # GPIO abstraction
│       └── Makefile
```

### 4.2 Protocol Compatibility

The CH32V003 port MUST implement:
- Same USB VID/PID (or configurable)
- Same 35 command protocol
- Same response format (8-byte buffer)
- Same host library compatibility

### 4.3 Hardware Abstraction Layer

Need to create `digital.h` for CH32V003:

```c
// ATtiny85 style
#define digitalWrite(port, pin, state) ...
#define pinMode(port, pin, mode) ...
#define digitalRead(port, pin) ...

// Mapped to CH32V003 registers
// GPIOA, GPIOC, GPIOD
```

---

## 5. Technical Challenges

### 5.1 USB Request Handling

**ATtiny85 (V-USB)**:
- `usbFunctionSetup()` - Handle SETUP packets
- `usbFunctionRead()` - Send response
- `usbFunctionWrite()` - Receive data

**CH32V003 (rv003usb)**:
- `usb_handle_other_control_message()` - Custom control
- `usb_send_data()` - Send response

**Solution**: Implement Little-Wire protocol handler in rv003usb callback model.

### 5.2 Timing-Critical Operations

| Operation | ATtiny85 Timing | CH32V003 Approach |
|-----------|-----------------|-------------------|
| WS2812 | 800kHz bitbang | Faster clock, easier timing |
| OneWire | µs precision | Same approach, adjust delays |
| SPI | USI or bitbang | Use hardware SPI |
| I2C | Bitbang | Use hardware I2C |

### 5.3 Pin Mapping Translation

| Little-Wire Pin | ATtiny85 | CH32V003 (proposed) |
|-----------------|----------|---------------------|
| PIN1 | PB1 | PC1 |
| PIN2 | PB2 | PC2 |
| PIN3 | PB5 | PC3 |
| PIN4 | PB0 | PC4 |
| USB D+ | PB4 | PD3 |
| USB D- | PB3 | PD4 |
| USB DPU | - | PD5 |

### 5.4 Peripheral Mapping

| Peripheral | ATtiny85 | CH32V003 |
|------------|----------|----------|
| ADC | ADC0-1 on PB5, PB2 | ADC channels on multiple pins |
| PWM | Timer0 OC0A/B | TIM1 CH1-4 |
| SPI | USI | Hardware SPI |
| I2C | Bitbang | Hardware I2C |

---

## 6. Benefits of CH32V003 Port

### 6.1 Cost Reduction
- CH32V003: ~$0.10 vs ATtiny85: ~$1.50
- **15x cost reduction**

### 6.2 Performance Improvement
- 3x clock speed (48MHz vs 16.5MHz)
- 32-bit vs 8-bit operations
- Hardware I2C/SPI vs bitbang
- 4x RAM (2KB vs 512B)

### 6.3 Feature Expansion
- More GPIO pins (18 vs 6)
- More ADC channels
- More PWM channels
- Potential for additional features

### 6.4 Supply Chain
- CH32V003 widely available
- Multiple package options (SOP8, TSSOP20, QFN20)

---

## 7. Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| USB timing compatibility | High | Use rv003usb proven implementation |
| Protocol mismatch | Medium | Thorough testing with host library |
| GPIO mapping confusion | Low | Clear documentation |
| Clock calibration | Medium | CH32V003 has better clock accuracy |
| Bootloader complexity | Low | rv003usb has working bootloader |

---

## 8. Recommended Pin Configuration (CH32V003J4M6 - SOP8)

For minimal SOP8 package (same as ATtiny85):

| Pin | CH32V003 | Function | Little-Wire |
|-----|----------|----------|-------------|
| 1 | PD4 | USB D- | USB |
| 2 | PD5 | USB DPU | Pull-up |
| 3 | PD6 | GPIO | PIN1 |
| 4 | VSS | GND | - |
| 5 | PD7/NRST | GPIO/Reset | PIN2 |
| 6 | PA1 | GPIO | PIN3 |
| 7 | PA2 | GPIO | PIN4 |
| 8 | VDD | 3.3V | - |

**Note**: USB D+ would need to be on PD3, requiring TSSOP20 or QFN20 package for full functionality.

---

## 9. Conclusion

The CH32V003 port is highly feasible using rv003usb as a submodule. Key advantages:
- 15x cost reduction
- Better performance
- More features
- Minimal code changes to host library

The main work involves:
1. Creating firmware abstraction layer
2. Implementing Little-Wire protocol on rv003usb
3. Adapting peripheral drivers
4. Testing all 35 commands

---

*Document version: 1.0*
*Date: 2025-11-27*
