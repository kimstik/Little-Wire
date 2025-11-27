# Little-Wire CH32V003 Port - Detailed Integration Plan

```
@author kimstik
assisted by intelligence
```

## Overview

This document describes the step-by-step integration plan for porting Little-Wire to CH32V003 using rv003usb as a git submodule.

---

## Phase 1: Repository Structure Setup

### 1.1 Create Directory Structure

```bash
firmware/
├── attiny85/                    # Move existing code here
│   ├── main.c
│   ├── digital.h
│   ├── usbconfig.h
│   ├── Makefile
│   └── usbdrv/
│
└── ch32v003/                    # New CH32V003 port
    ├── rv003usb/                # Git submodule → cnlohr/rv003usb
    ├── main.c                   # Little-Wire CH32V003 firmware
    ├── usb_config.h             # rv003usb configuration
    ├── digital.h                # GPIO abstraction layer
    ├── littlewire_protocol.h    # Protocol definitions
    ├── peripherals/
    │   ├── gpio.c/h
    │   ├── adc.c/h
    │   ├── pwm.c/h
    │   ├── spi.c/h
    │   ├── i2c.c/h
    │   ├── onewire.c/h
    │   └── ws2812.c/h
    └── Makefile
```

### 1.2 Add Git Submodule

```bash
cd firmware/ch32v003
git submodule add https://github.com/cnlohr/rv003usb.git rv003usb
git submodule update --init --recursive
```

This will pull rv003usb along with its dependency (ch32v003fun).

---

## Phase 2: USB Configuration

### 2.1 usb_config.h

```c
/**
 * @file usb_config.h
 * @brief Little-Wire USB configuration for rv003usb
 * @author kimstik
 * assisted by intelligence
 */

#ifndef USB_CONFIG_H
#define USB_CONFIG_H

// Number of endpoints (EP0 + EP1 interrupt)
#define ENDPOINTS 2

// USB Pin Configuration (CH32V003 TSSOP20)
#define USB_PORT   D
#define USB_PIN_DP 3
#define USB_PIN_DM 4
#define USB_PIN_DPU 5  // 1.5k pull-up control

// Feature flags
#define RV003USB_HANDLE_IN_REQUEST      0
#define RV003USB_OTHER_CONTROL          1  // Enable for Little-Wire protocol
#define RV003USB_HANDLE_USER_DATA       0
#define RV003USB_HID_FEATURES           0
#define RV003USB_OPTIMIZE_FLASH         0

// USB Device Descriptor
#define USB_VID 0x1781   // Same as original Little-Wire
#define USB_PID 0x0c9f   // Same as original Little-Wire
#define USB_REV 0x0200   // Version 2.0 (CH32V003)

// String descriptors
#define USB_MANUFACTURER "Little-Wire"
#define USB_PRODUCT      "Little-Wire CH32V003"
#define USB_SERIAL       "001"

#endif // USB_CONFIG_H
```

### 2.2 USB Descriptor Structure

```c
// Device descriptor - vendor class (0xFF)
static const uint8_t device_descriptor[] = {
    18,         // bLength
    1,          // bDescriptorType (Device)
    0x10, 0x01, // bcdUSB 1.1
    0xFF,       // bDeviceClass (Vendor)
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    8,          // bMaxPacketSize0
    USB_VID & 0xFF, USB_VID >> 8,  // idVendor
    USB_PID & 0xFF, USB_PID >> 8,  // idProduct
    USB_REV & 0xFF, USB_REV >> 8,  // bcdDevice
    1,          // iManufacturer
    2,          // iProduct
    3,          // iSerialNumber
    1           // bNumConfigurations
};
```

---

## Phase 3: Protocol Layer

### 3.1 littlewire_protocol.h

```c
/**
 * @file littlewire_protocol.h
 * @brief Little-Wire USB protocol definitions
 * @author kimstik
 * assisted by intelligence
 */

#ifndef LITTLEWIRE_PROTOCOL_H
#define LITTLEWIRE_PROTOCOL_H

#define LITTLE_WIRE_VERSION 0x20  // Version 2.0 for CH32V003

// Generic requests (0-4) - USBtiny compatible
#define USBTINY_ECHO              0
#define USBTINY_READ              1
#define USBTINY_WRITE             2
#define USBTINY_CLR               3
#define USBTINY_SET               4

// Programming requests (5-12) - AVR ISP compatible
#define USBTINY_POWERUP           5
#define USBTINY_POWERDOWN         6
#define USBTINY_SPI               7
#define USBTINY_POLL_BYTES        8
#define USBTINY_FLASH_READ        9
#define USBTINY_FLASH_WRITE       10
#define USBTINY_EEPROM_READ       11
#define USBTINY_EEPROM_WRITE      12

// GPIO requests (13-20)
#define USBTINY_PIN_SET_INPUT     13
#define USBTINY_PIN_SET_OUTPUT    14
#define USBTINY_READ_ADC          15
#define USBTINY_SETUP_PWM         16
#define USBTINY_UPDATE_PWM_COMPARE 17
#define USBTINY_PIN_SET_HIGH      18
#define USBTINY_PIN_SET_LOW       19
#define USBTINY_PIN_READ          20

// SPI requests (21, 23, 28-29, 31, 33)
#define USBTINY_SINGLE_SPI        21
#define USBTINY_CHANGE_PWM_PRESCALE 22
#define USBTINY_SETUP_SPI         23
#define USBTINY_SPI_ADD_BUFFER    28
#define USBTINY_SPI_SEND_BUFFER   29
#define USBTINY_SPI_UPDATE_DELAY  31
#define USBTINY_DEBUG_SPI         33

// I2C requests (24-27, 30, 49)
#define USBTINY_SETUP_I2C         24
#define USBTINY_I2C_BEGIN_TX      25
#define USBTINY_I2C_ADD_BUFFER    26
#define USBTINY_I2C_SEND_BUFFER   27
#define USBTINY_I2C_REQUEST_FROM  30
#define USBTINY_I2C_UPDATE_DELAY  49

// System requests (32, 34-35, 40, 55)
#define USBTINY_STOP_PWM          32
#define USBTINY_VERSION_QUERY     34
#define USBTINY_INIT_ADC          35
#define USBTINY_READ_BUFFER       40
#define USBTINY_CHANGE_SERIAL     55

// OneWire requests (41-43, 50-51)
#define USBTINY_ONEWIRE_RESET     41
#define USBTINY_ONEWIRE_SEND_BYTE 42
#define USBTINY_ONEWIRE_READ_BYTE 43
#define USBTINY_SOFTPWM_INIT      47
#define USBTINY_SOFTPWM_UPDATE    48
#define USBTINY_ONEWIRE_READ_BIT  50
#define USBTINY_ONEWIRE_WRITE_BIT 51

// WS2812 requests (54)
#define USBTINY_WS2812_WRITE      54

// Multi-byte I2C (0xE0-0xEF)
#define USBTINY_I2C_MULTI_BASE    0xE0

// Multi-byte SPI (0xF0-0xFF)
#define USBTINY_SPI_MULTI_BASE    0xF0

#endif // LITTLEWIRE_PROTOCOL_H
```

### 3.2 Protocol Handler Integration

```c
/**
 * @brief Handle Little-Wire control messages via rv003usb
 *
 * Called from rv003usb when RV003USB_OTHER_CONTROL is enabled
 */
void usb_handle_other_control_message(void) {
    struct usb_urb* urb = &rv003usb_internal_data.ctrl_req;
    uint8_t req = (urb->wRequestTypeLSBRequestMSB >> 8) & 0xFF;
    uint16_t wValue = urb->lValueLSBIndexMSB & 0xFFFF;
    uint16_t wIndex = (urb->lValueLSBIndexMSB >> 16) & 0xFFFF;

    // Route to appropriate handler
    switch(req) {
        case USBTINY_ECHO:
            handle_echo(wValue, wIndex);
            break;
        case USBTINY_PIN_SET_INPUT:
        case USBTINY_PIN_SET_OUTPUT:
        case USBTINY_PIN_SET_HIGH:
        case USBTINY_PIN_SET_LOW:
        case USBTINY_PIN_READ:
            handle_gpio(req, wValue, wIndex);
            break;
        // ... other handlers
    }
}
```

---

## Phase 4: Hardware Abstraction Layer

### 4.1 digital.h (CH32V003 version)

```c
/**
 * @file digital.h
 * @brief GPIO abstraction macros for CH32V003
 * @author kimstik
 * assisted by intelligence
 */

#ifndef DIGITAL_H
#define DIGITAL_H

#include "ch32v003fun.h"

// Pin modes
#define INPUT       0
#define OUTPUT      1
#define INPUT_PULLUP 2

// Pin states
#define LOW         0
#define HIGH        1
#define ENABLE      1
#define DISABLE     0

// GPIO port base addresses
#define GPIOA_BASE  ((GPIO_TypeDef*)0x40010800)
#define GPIOC_BASE  ((GPIO_TypeDef*)0x40011000)
#define GPIOD_BASE  ((GPIO_TypeDef*)0x40011400)

// Port selection macro
#define GPIO_PORT(port) GPIO##port##_BASE

// Pin mode configuration
#define pinMode(port, pin, mode) do { \
    GPIO_TypeDef* gpio = GPIO_PORT(port); \
    uint32_t cfg = gpio->CFGLR; \
    cfg &= ~(0xF << ((pin) * 4)); \
    if ((mode) == OUTPUT) { \
        cfg |= (0x1 << ((pin) * 4)); /* Push-pull 10MHz */ \
    } else if ((mode) == INPUT_PULLUP) { \
        cfg |= (0x8 << ((pin) * 4)); /* Input with pull-up */ \
        gpio->BSHR = (1 << (pin)); \
    } else { \
        cfg |= (0x4 << ((pin) * 4)); /* Floating input */ \
    } \
    gpio->CFGLR = cfg; \
} while(0)

// Digital write
#define digitalWrite(port, pin, state) do { \
    GPIO_TypeDef* gpio = GPIO_PORT(port); \
    if (state) { \
        gpio->BSHR = (1 << (pin)); \
    } else { \
        gpio->BCR = (1 << (pin)); \
    } \
} while(0)

// Digital read
#define digitalRead(port, pin) \
    ((GPIO_PORT(port)->INDR >> (pin)) & 1)

// Internal pullup (CH32V003 - configure via mode)
#define internalPullup(port, pin, state) do { \
    if (state) { \
        pinMode(port, pin, INPUT_PULLUP); \
    } else { \
        pinMode(port, pin, INPUT); \
    } \
} while(0)

// Toggle pin
#define togglePin(port, pin) do { \
    GPIO_TypeDef* gpio = GPIO_PORT(port); \
    gpio->OUTDR ^= (1 << (pin)); \
} while(0)

#endif // DIGITAL_H
```

### 4.2 Pin Mapping

```c
/**
 * @file pin_mapping.h
 * @brief Little-Wire pin mapping for CH32V003
 * @author kimstik
 * assisted by intelligence
 */

#ifndef PIN_MAPPING_H
#define PIN_MAPPING_H

// Little-Wire logical pins -> CH32V003 physical pins
// Using TSSOP20 package for maximum GPIO

// Main GPIO pins (exposed to user)
#define PIN1_PORT   C
#define PIN1_PIN    1

#define PIN2_PORT   C
#define PIN2_PIN    2

#define PIN3_PORT   C
#define PIN3_PIN    3

#define PIN4_PORT   C
#define PIN4_PIN    4

// Additional pins (CH32V003 bonus)
#define PIN5_PORT   C
#define PIN5_PIN    5

#define PIN6_PORT   C
#define PIN6_PIN    6

// USB pins (fixed)
#define USB_DP_PORT D
#define USB_DP_PIN  3

#define USB_DM_PORT D
#define USB_DM_PIN  4

#define USB_DPU_PORT D
#define USB_DPU_PIN  5

// Peripheral mappings
#define SPI_MOSI_PORT C
#define SPI_MOSI_PIN  6

#define SPI_MISO_PORT C
#define SPI_MISO_PIN  7

#define SPI_SCK_PORT  C
#define SPI_SCK_PIN   5

#define I2C_SDA_PORT  C
#define I2C_SDA_PIN   1

#define I2C_SCL_PORT  C
#define I2C_SCL_PIN   2

// ADC channels (CH32V003 has ADC on multiple pins)
#define ADC_CH0_PORT  A
#define ADC_CH0_PIN   2

#define ADC_CH1_PORT  A
#define ADC_CH1_PIN   1

// OneWire default
#define ONEWIRE_PORT  C
#define ONEWIRE_PIN   4

// WS2812 default
#define WS2812_PORT   C
#define WS2812_PIN    1

#endif // PIN_MAPPING_H
```

---

## Phase 5: Peripheral Drivers

### 5.1 ADC Driver

```c
/**
 * @file adc.c
 * @brief ADC driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32v003fun.h"
#include "adc.h"

static uint8_t adc_prescaler = 0;
static uint8_t adc_reference = 0;

void adc_init(uint8_t prescaler, uint8_t reference) {
    // Enable ADC clock
    RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

    // Configure ADC
    ADC1->CTLR2 = ADC_ADON;  // Power on

    // Set prescaler (ADCPRE in RCC_CFGR0)
    RCC->CFGR0 &= ~RCC_ADCPRE;
    RCC->CFGR0 |= (prescaler & 0x03) << 14;

    adc_prescaler = prescaler;
    adc_reference = reference;

    // Calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while(ADC1->CTLR2 & ADC_RSTCAL);
    ADC1->CTLR2 |= ADC_CAL;
    while(ADC1->CTLR2 & ADC_CAL);
}

uint16_t adc_read(uint8_t channel) {
    // Configure channel
    ADC1->RSQR3 = channel;

    // Start conversion
    ADC1->CTLR2 |= ADC_SWSTART;

    // Wait for completion
    while(!(ADC1->STATR & ADC_EOC));

    // Return 10-bit result
    return ADC1->RDATAR & 0x3FF;
}
```

### 5.2 PWM Driver

```c
/**
 * @file pwm.c
 * @brief PWM driver using TIM1 for CH32V003
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32v003fun.h"
#include "pwm.h"

static uint8_t pwm_prescaler_index = 4;  // Default: /1024

// Prescaler values matching ATtiny85
static const uint16_t prescaler_values[] = {
    1,      // 0: /1
    8,      // 1: /8
    64,     // 2: /64
    256,    // 3: /256
    1024    // 4: /1024 (default)
};

void pwm_init(void) {
    // Enable TIM1 clock
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;

    // Configure GPIO for PWM output (PC1, PC2 for TIM1_CH1, TIM1_CH2)
    // ... GPIO alternate function setup

    // Configure TIM1
    TIM1->PSC = prescaler_values[pwm_prescaler_index] - 1;
    TIM1->ATRLR = 255;  // 8-bit resolution

    // PWM mode 1 on CH1 and CH2
    TIM1->CHCTLR1 = 0x6060;  // PWM mode 1, preload enable
    TIM1->CCER = TIM_CC1E | TIM_CC2E;  // Enable outputs
    TIM1->BDTR = TIM_MOE;  // Main output enable

    TIM1->CTLR1 = TIM_CEN;  // Enable timer
}

void pwm_set_compare(uint8_t ch_a, uint8_t ch_b) {
    TIM1->CH1CVR = ch_a;
    TIM1->CH2CVR = ch_b;
}

void pwm_set_prescaler(uint8_t index) {
    if (index > 4) index = 4;
    pwm_prescaler_index = index;
    TIM1->PSC = prescaler_values[index] - 1;
}

void pwm_stop(void) {
    TIM1->CTLR1 = 0;
}
```

### 5.3 SPI Driver (Hardware)

```c
/**
 * @file spi.c
 * @brief Hardware SPI driver for CH32V003
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32v003fun.h"
#include "spi.h"

static uint16_t spi_delay = 0;

void spi_init(void) {
    // Enable SPI1 clock
    RCC->APB2PCENR |= RCC_APB2Periph_SPI1;

    // Configure GPIO (PC5=SCK, PC6=MOSI, PC7=MISO)
    // ... GPIO alternate function setup

    // Configure SPI: Master, CPOL=0, CPHA=0, /16
    SPI1->CTLR1 = SPI_MSTR | SPI_BR_16 | SPI_SSM | SPI_SSI;
    SPI1->CTLR1 |= SPI_SPE;  // Enable
}

uint8_t spi_transfer(uint8_t data) {
    while(!(SPI1->STATR & SPI_TXE));
    SPI1->DATAR = data;
    while(!(SPI1->STATR & SPI_RXNE));
    return SPI1->DATAR;
}

void spi_transfer_buffer(uint8_t* tx, uint8_t* rx, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        rx[i] = spi_transfer(tx[i]);
        if (spi_delay > 0) {
            Delay_Us(spi_delay);
        }
    }
}

void spi_set_delay(uint16_t delay_us) {
    spi_delay = delay_us;
}
```

### 5.4 I2C Driver (Hardware)

```c
/**
 * @file i2c.c
 * @brief Hardware I2C driver for CH32V003
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32v003fun.h"
#include "i2c.h"

static uint8_t i2c_delay = 5;

void i2c_init(void) {
    // Enable I2C1 clock
    RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

    // Configure GPIO (PC1=SDA, PC2=SCL)
    // ... GPIO alternate function setup (open-drain)

    // Configure I2C: 100kHz standard mode
    I2C1->CTLR2 = 48;  // 48MHz peripheral clock
    I2C1->CKCFGR = 240;  // 100kHz = 48MHz / (2 * 240)
    I2C1->CTLR1 = I2C_PE;  // Enable
}

uint8_t i2c_start(uint8_t address) {
    // Generate START
    I2C1->CTLR1 |= I2C_START;
    while(!(I2C1->STAR1 & I2C_SB));

    // Send address
    I2C1->DATAR = address;
    while(!(I2C1->STAR1 & (I2C_ADDR | I2C_AF)));

    if (I2C1->STAR1 & I2C_AF) {
        I2C1->STAR1 &= ~I2C_AF;
        return 1;  // NACK
    }

    // Clear ADDR flag
    (void)I2C1->STAR2;
    return 0;  // ACK
}

uint8_t i2c_write(uint8_t data) {
    I2C1->DATAR = data;
    while(!(I2C1->STAR1 & I2C_TXE));
    return (I2C1->STAR1 & I2C_AF) ? 1 : 0;
}

uint8_t i2c_read(uint8_t ack) {
    if (ack) {
        I2C1->CTLR1 |= I2C_ACK;
    } else {
        I2C1->CTLR1 &= ~I2C_ACK;
    }

    while(!(I2C1->STAR1 & I2C_RXNE));
    return I2C1->DATAR;
}

void i2c_stop(void) {
    I2C1->CTLR1 |= I2C_STOP;
}

void i2c_set_delay(uint8_t delay) {
    i2c_delay = delay;
    // Reconfigure I2C speed based on delay
}
```

### 5.5 OneWire Driver

```c
/**
 * @file onewire.c
 * @brief OneWire bitbang driver for CH32V003
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32v003fun.h"
#include "onewire.h"
#include "digital.h"

#define OW_PORT ONEWIRE_PORT
#define OW_PIN  ONEWIRE_PIN

uint8_t onewire_reset(void) {
    uint8_t presence;

    // Drive low for 480us
    pinMode(OW_PORT, OW_PIN, OUTPUT);
    digitalWrite(OW_PORT, OW_PIN, LOW);
    Delay_Us(480);

    // Release and wait 70us
    __disable_irq();
    digitalWrite(OW_PORT, OW_PIN, HIGH);
    Delay_Us(70);

    // Sample presence
    pinMode(OW_PORT, OW_PIN, INPUT);
    presence = !digitalRead(OW_PORT, OW_PIN);
    __enable_irq();

    Delay_Us(410);

    // Release bus
    pinMode(OW_PORT, OW_PIN, OUTPUT);
    digitalWrite(OW_PORT, OW_PIN, HIGH);

    return presence;
}

void onewire_write_byte(uint8_t data) {
    pinMode(OW_PORT, OW_PIN, OUTPUT);

    for (uint8_t i = 0; i < 8; i++) {
        __disable_irq();
        digitalWrite(OW_PORT, OW_PIN, LOW);

        if (data & 0x01) {
            Delay_Us(6);
            digitalWrite(OW_PORT, OW_PIN, HIGH);
            Delay_Us(64);
        } else {
            Delay_Us(60);
            digitalWrite(OW_PORT, OW_PIN, HIGH);
            Delay_Us(10);
        }
        __enable_irq();

        data >>= 1;
    }
}

uint8_t onewire_read_byte(void) {
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;

        pinMode(OW_PORT, OW_PIN, OUTPUT);

        __disable_irq();
        digitalWrite(OW_PORT, OW_PIN, LOW);
        Delay_Us(6);
        digitalWrite(OW_PORT, OW_PIN, HIGH);
        Delay_Us(10);

        pinMode(OW_PORT, OW_PIN, INPUT);
        if (digitalRead(OW_PORT, OW_PIN)) {
            data |= 0x80;
        }
        __enable_irq();

        Delay_Us(55);
    }

    return data;
}

void onewire_write_bit(uint8_t bit) {
    pinMode(OW_PORT, OW_PIN, OUTPUT);

    __disable_irq();
    digitalWrite(OW_PORT, OW_PIN, LOW);

    if (bit & 0x01) {
        Delay_Us(6);
        digitalWrite(OW_PORT, OW_PIN, HIGH);
        Delay_Us(64);
    } else {
        Delay_Us(60);
        digitalWrite(OW_PORT, OW_PIN, HIGH);
        Delay_Us(10);
    }
    __enable_irq();
}

uint8_t onewire_read_bit(void) {
    uint8_t bit;

    pinMode(OW_PORT, OW_PIN, OUTPUT);

    __disable_irq();
    digitalWrite(OW_PORT, OW_PIN, LOW);
    Delay_Us(6);
    digitalWrite(OW_PORT, OW_PIN, HIGH);
    Delay_Us(10);

    pinMode(OW_PORT, OW_PIN, INPUT);
    bit = digitalRead(OW_PORT, OW_PIN);
    __enable_irq();

    return bit;
}
```

### 5.6 WS2812 Driver

```c
/**
 * @file ws2812.c
 * @brief WS2812 LED driver for CH32V003 (optimized bitbang)
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32v003fun.h"
#include "ws2812.h"
#include "digital.h"

#define WS2812_MAX_LEDS 64

static uint8_t ws2812_buffer[WS2812_MAX_LEDS * 3];
static uint8_t ws2812_ptr = 0;
static uint8_t ws2812_pin = WS2812_PIN;
static GPIO_TypeDef* ws2812_gpio = GPIOC;

void ws2812_preload(uint8_t r, uint8_t g, uint8_t b) {
    if (ws2812_ptr < WS2812_MAX_LEDS * 3) {
        ws2812_buffer[ws2812_ptr++] = g;  // GRB order
        ws2812_buffer[ws2812_ptr++] = r;
        ws2812_buffer[ws2812_ptr++] = b;
    }
}

// Timing at 48MHz:
// T0H = 0.35us = 17 cycles
// T0L = 0.80us = 38 cycles
// T1H = 0.70us = 34 cycles
// T1L = 0.60us = 29 cycles

void ws2812_flush(uint8_t pin) {
    uint32_t mask_hi = (1 << pin);
    uint32_t mask_lo = ~mask_hi;

    __disable_irq();

    for (uint16_t i = 0; i < ws2812_ptr; i++) {
        uint8_t byte = ws2812_buffer[i];

        for (uint8_t bit = 0x80; bit; bit >>= 1) {
            if (byte & bit) {
                // Send '1': high 0.7us, low 0.6us
                ws2812_gpio->BSHR = mask_hi;
                __asm__ volatile("nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;");
                ws2812_gpio->BCR = mask_hi;
                __asm__ volatile("nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;");
            } else {
                // Send '0': high 0.35us, low 0.8us
                ws2812_gpio->BSHR = mask_hi;
                __asm__ volatile("nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;");
                ws2812_gpio->BCR = mask_hi;
                __asm__ volatile("nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;"
                                 "nop;nop;nop;nop;nop;nop;nop;nop;");
            }
        }
    }

    __enable_irq();

    ws2812_ptr = 0;  // Reset buffer
}
```

---

## Phase 6: Main Firmware

### 6.1 main.c Structure

```c
/**
 * @file main.c
 * @brief Little-Wire CH32V003 firmware
 * @author kimstik
 * assisted by intelligence
 *
 * Based on Little-Wire by ihsan Kehribar and chris chung
 * USB implementation via rv003usb by CNLohr
 */

#include "ch32v003fun.h"
#include "rv003usb.h"
#include "littlewire_protocol.h"
#include "digital.h"
#include "pin_mapping.h"

// Peripheral drivers
#include "peripherals/adc.h"
#include "peripherals/pwm.h"
#include "peripherals/spi.h"
#include "peripherals/i2c.h"
#include "peripherals/onewire.h"
#include "peripherals/ws2812.h"

// State buffers
static volatile uint8_t sendBuffer[9];
static volatile uint8_t rxBuffer[8];
static volatile uint8_t jobState = 0;

// rv003usb callback
void usb_handle_other_control_message(void) {
    // Get request from rv003usb
    struct usb_urb* urb = &rv003usb_internal_data.ctrl_req;
    uint8_t req = (urb->wRequestTypeLSBRequestMSB >> 8) & 0xFF;
    uint16_t wValue = urb->lValueLSBIndexMSB & 0xFFFF;
    uint16_t wIndex = (urb->lValueLSBIndexMSB >> 16) & 0xFFFF;
    uint8_t bit = wValue & 7;

    // Handle Little-Wire protocol
    switch(req) {
        case USBTINY_ECHO:
            // Echo test
            usb_send_data((uint8_t*)&wValue, 2, 0, 0);
            break;

        case USBTINY_PIN_SET_INPUT:
            set_pin_input(bit);
            usb_send_empty(0);
            break;

        case USBTINY_PIN_SET_OUTPUT:
            set_pin_output(bit);
            usb_send_empty(0);
            break;

        case USBTINY_PIN_SET_HIGH:
            set_pin_high(bit);
            usb_send_empty(0);
            break;

        case USBTINY_PIN_SET_LOW:
            set_pin_low(bit);
            usb_send_empty(0);
            break;

        case USBTINY_PIN_READ:
            sendBuffer[0] = read_pin(bit);
            usb_send_data((uint8_t*)sendBuffer, 1, 0, 0);
            break;

        case USBTINY_READ_ADC:
            {
                uint16_t adc_val = adc_read(wValue & 0xFF);
                sendBuffer[0] = adc_val & 0xFF;
                sendBuffer[1] = (adc_val >> 8) & 0xFF;
                usb_send_data((uint8_t*)sendBuffer, 2, 0, 0);
            }
            break;

        case USBTINY_VERSION_QUERY:
            sendBuffer[0] = LITTLE_WIRE_VERSION;
            usb_send_data((uint8_t*)sendBuffer, 1, 0, 0);
            break;

        // ... implement all other commands

        default:
            usb_send_empty(0);
            break;
    }
}

int main(void) {
    // System init
    SystemInit();

    // Enable GPIO clocks
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA |
                      RCC_APB2Periph_GPIOC |
                      RCC_APB2Periph_GPIOD;

    // Initialize USB
    usb_setup();

    // Initialize peripherals in low-power state
    // ...

    // Main loop
    while(1) {
        // Process any pending job states
        process_job_state();
    }

    return 0;
}
```

---

## Phase 7: Build System

### 7.1 Makefile

```makefile
# Little-Wire CH32V003 Makefile
# @author kimstik
# assisted by intelligence

TARGET = littlewire_ch32v003

# Toolchain
PREFIX = riscv-none-embed-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

# Paths
RV003USB = rv003usb
CH32V003FUN = $(RV003USB)/ch32v003fun

# Sources
SRCS = main.c
SRCS += peripherals/adc.c
SRCS += peripherals/pwm.c
SRCS += peripherals/spi.c
SRCS += peripherals/i2c.c
SRCS += peripherals/onewire.c
SRCS += peripherals/ws2812.c
SRCS += $(RV003USB)/rv003usb/rv003usb.c

# Assembly
ASRC = $(RV003USB)/rv003usb/rv003usb.S
ASRC += $(CH32V003FUN)/ch32v003fun/ch32v003fun.S

# Includes
INCLUDES = -I.
INCLUDES += -I$(RV003USB)/rv003usb
INCLUDES += -I$(CH32V003FUN)/ch32v003fun
INCLUDES += -Iperipherals

# Flags
CFLAGS = -march=rv32ec -mabi=ilp32e
CFLAGS += -Os -flto -ffunction-sections -fdata-sections
CFLAGS += -Wall -Wextra
CFLAGS += $(INCLUDES)

LDFLAGS = -T $(CH32V003FUN)/ch32v003fun/ch32v003fun.ld
LDFLAGS += -Wl,--gc-sections -nostdlib

# Objects
OBJS = $(SRCS:.c=.o) $(ASRC:.S=.o)

all: $(TARGET).bin size

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

size: $(TARGET).elf
	$(SIZE) $<

flash: $(TARGET).bin
	minichlink -w $< flash -b

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin

.PHONY: all clean flash size
```

---

## Phase 8: Testing Plan

### 8.1 Unit Tests

| Test | Description | Pass Criteria |
|------|-------------|---------------|
| USB Enumeration | Device appears on USB bus | VID:PID correct |
| Version Query | Request 34 returns version | Returns 0x20 |
| GPIO Output | Set pin high/low | Pin voltage correct |
| GPIO Input | Read pin state | Correct level detected |
| ADC Read | Read analog value | ±10% accuracy |
| PWM Output | Generate PWM signal | Frequency ±5% |
| SPI Transfer | Send/receive data | Echo correct |
| I2C Transfer | Communicate with sensor | ACK received |
| OneWire Reset | Detect DS18B20 | Presence detected |
| WS2812 | Light LED | Correct color |

### 8.2 Integration Tests

Test with existing host library:
```c
// Test program
#include "littleWire.h"

int main() {
    littleWire* lw = littleWire_connect();

    printf("Firmware version: 0x%02X\n", readFirmwareVersion(lw));

    // Test GPIO
    pinMode(lw, PIN1, OUTPUT);
    digitalWrite(lw, PIN1, HIGH);
    delay(500);
    digitalWrite(lw, PIN1, LOW);

    // Test ADC
    analog_init(lw, VREF_VCC);
    printf("ADC: %d\n", analogRead(lw, 0));

    // Test I2C
    i2c_init(lw);
    i2c_start(lw, 0x50, WRITE);
    // ...

    return 0;
}
```

---

## Phase 9: Timeline and Milestones

### Milestone 1: Project Setup
- [ ] Create directory structure
- [ ] Add rv003usb submodule
- [ ] Basic Makefile working

### Milestone 2: USB Working
- [ ] USB enumeration
- [ ] Version query response
- [ ] Echo test working

### Milestone 3: GPIO
- [ ] Digital input/output
- [ ] Pull-up configuration
- [ ] Pin read

### Milestone 4: Analog/PWM
- [ ] ADC initialization
- [ ] ADC read channels
- [ ] PWM initialization
- [ ] PWM compare update
- [ ] PWM prescaler

### Milestone 5: Serial Protocols
- [ ] SPI master mode
- [ ] I2C master mode
- [ ] Debug SPI

### Milestone 6: Special Protocols
- [ ] OneWire reset
- [ ] OneWire byte transfer
- [ ] OneWire bit transfer

### Milestone 7: LED Support
- [ ] WS2812 preload
- [ ] WS2812 flush

### Milestone 8: Testing & Documentation
- [ ] Full protocol test
- [ ] Host library compatibility
- [ ] Documentation update

---

## Appendix A: Command Implementation Checklist

| ID | Command | Priority | Status |
|----|---------|----------|--------|
| 0 | ECHO | High | [ ] |
| 1 | READ | Low | [ ] |
| 2 | WRITE | Low | [ ] |
| 3 | CLR | Low | [ ] |
| 4 | SET | Low | [ ] |
| 5 | POWERUP | Medium | [ ] |
| 6 | POWERDOWN | Medium | [ ] |
| 7 | SPI | Medium | [ ] |
| 13 | PIN_SET_INPUT | High | [ ] |
| 14 | PIN_SET_OUTPUT | High | [ ] |
| 15 | READ_ADC | High | [ ] |
| 16 | SETUP_PWM | High | [ ] |
| 17 | UPDATE_PWM_COMPARE | High | [ ] |
| 18 | PIN_SET_HIGH | High | [ ] |
| 19 | PIN_SET_LOW | High | [ ] |
| 20 | PIN_READ | High | [ ] |
| 21 | SINGLE_SPI | Medium | [ ] |
| 22 | CHANGE_PWM_PRESCALE | Medium | [ ] |
| 23 | SETUP_SPI | Medium | [ ] |
| 24 | SETUP_I2C | High | [ ] |
| 25 | I2C_BEGIN_TX | High | [ ] |
| 26 | I2C_ADD_BUFFER | High | [ ] |
| 27 | I2C_SEND_BUFFER | High | [ ] |
| 28 | SPI_ADD_BUFFER | Medium | [ ] |
| 29 | SPI_SEND_BUFFER | Medium | [ ] |
| 30 | I2C_REQUEST_FROM | High | [ ] |
| 31 | SPI_UPDATE_DELAY | Low | [ ] |
| 32 | STOP_PWM | Medium | [ ] |
| 33 | DEBUG_SPI | Low | [ ] |
| 34 | VERSION_QUERY | High | [ ] |
| 35 | INIT_ADC | High | [ ] |
| 40 | READ_BUFFER | Medium | [ ] |
| 41 | ONEWIRE_RESET | Medium | [ ] |
| 42 | ONEWIRE_SEND_BYTE | Medium | [ ] |
| 43 | ONEWIRE_READ_BYTE | Medium | [ ] |
| 44 | I2C_INIT | High | [ ] |
| 45 | I2C_BEGIN | High | [ ] |
| 46 | I2C_READ | High | [ ] |
| 47 | SOFTPWM_INIT | Low | [ ] |
| 48 | SOFTPWM_UPDATE | Low | [ ] |
| 49 | I2C_UPDATE_DELAY | Low | [ ] |
| 50 | ONEWIRE_READ_BIT | Medium | [ ] |
| 51 | ONEWIRE_WRITE_BIT | Medium | [ ] |
| 54 | WS2812_WRITE | Medium | [ ] |
| 55 | CHANGE_SERIAL | Low | [ ] |
| 0xE0-0xEF | I2C_MULTI | Medium | [ ] |
| 0xF0-0xFF | SPI_MULTI | Medium | [ ] |

---

*Document version: 1.0*
*Date: 2025-11-27*
