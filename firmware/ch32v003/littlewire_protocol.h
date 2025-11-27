/**
 * @file littlewire_protocol.h
 * @brief Little-Wire USB protocol definitions
 * @author kimstik
 * assisted by intelligence
 *
 * Protocol compatible with original Little-Wire firmware v1.3
 * Based on USBtiny protocol with Little-Wire extensions
 */

#ifndef _LITTLEWIRE_PROTOCOL_H
#define _LITTLEWIRE_PROTOCOL_H

// Firmware version: 0x20 = 2.0 (CH32V003 port)
#define LITTLE_WIRE_VERSION 0x20

// ==== Generic Requests (0-4) - USBtiny compatible ====
#define USBTINY_ECHO              0   // Echo test
#define USBTINY_READ              1   // Read byte from address
#define USBTINY_WRITE             2   // Write byte to address
#define USBTINY_CLR               3   // Clear bit at address
#define USBTINY_SET               4   // Set bit at address

// ==== Programming Requests (5-12) - AVR ISP compatible ====
#define USBTINY_POWERUP           5   // Power up target (set SPI period)
#define USBTINY_POWERDOWN         6   // Power down target
#define USBTINY_SPI               7   // SPI command (4 bytes)
#define USBTINY_POLL_BYTES        8   // Set poll bytes for write
#define USBTINY_FLASH_READ        9   // Read flash
#define USBTINY_FLASH_WRITE       10  // Write flash
#define USBTINY_EEPROM_READ       11  // Read EEPROM
#define USBTINY_EEPROM_WRITE      12  // Write EEPROM

// ==== GPIO Requests ====
#define USBTINY_PIN_SET_INPUT     13  // Set pin as input
#define USBTINY_PIN_SET_OUTPUT    14  // Set pin as output
#define USBTINY_READ_ADC          15  // Read ADC channel
#define USBTINY_SETUP_PWM         16  // Initialize PWM
#define USBTINY_UPDATE_PWM_COMPARE 17 // Update PWM compare values
#define USBTINY_PIN_SET_HIGH      18  // Set pin high
#define USBTINY_PIN_SET_LOW       19  // Set pin low
#define USBTINY_PIN_READ          20  // Read pin state

// ==== SPI Requests ====
#define USBTINY_SINGLE_SPI        21  // Single SPI byte transfer
#define USBTINY_CHANGE_PWM_PRESCALE 22 // Change PWM prescaler
#define USBTINY_SETUP_SPI         23  // Initialize SPI
#define USBTINY_SPI_ADD_BUFFER    28  // Add byte to SPI buffer
#define USBTINY_SPI_SEND_BUFFER   29  // Send SPI buffer
#define USBTINY_SPI_UPDATE_DELAY  31  // Update SPI delay
#define USBTINY_DEBUG_SPI         33  // Debug SPI (slow bitbang)

// ==== I2C Requests ====
#define USBTINY_SETUP_I2C         24  // Initialize I2C
#define USBTINY_I2C_BEGIN_TX      25  // I2C start + address
#define USBTINY_I2C_ADD_BUFFER    26  // Add byte to I2C buffer
#define USBTINY_I2C_SEND_BUFFER   27  // Send I2C buffer
#define USBTINY_I2C_REQUEST_FROM  30  // I2C read request

// ==== System Requests ====
#define USBTINY_STOP_PWM          32  // Stop PWM
#define USBTINY_VERSION_QUERY     34  // Query firmware version
#define USBTINY_INIT_ADC          35  // Initialize ADC
#define USBTINY_READ_BUFFER       40  // Read result buffer

// ==== OneWire Requests ====
#define USBTINY_ONEWIRE_RESET     41  // OneWire reset pulse
#define USBTINY_ONEWIRE_SEND_BYTE 42  // OneWire send byte
#define USBTINY_ONEWIRE_READ_BYTE 43  // OneWire read byte

// ==== I2C Extended Requests ====
#define USBTINY_I2C_INIT          44  // I2C init (alternative)
#define USBTINY_I2C_BEGIN         45  // I2C begin (alternative)
#define USBTINY_I2C_READ          46  // I2C read bytes

// ==== Software PWM Requests ====
#define USBTINY_SOFTPWM_INIT      47  // Initialize soft PWM
#define USBTINY_SOFTPWM_UPDATE    48  // Update soft PWM values

// ==== I2C/OneWire Extended ====
#define USBTINY_I2C_UPDATE_DELAY  49  // Update I2C delay
#define USBTINY_ONEWIRE_READ_BIT  50  // OneWire read single bit
#define USBTINY_ONEWIRE_WRITE_BIT 51  // OneWire write single bit

// ==== WS2812 Requests ====
#define USBTINY_WS2812_WRITE      54  // WS2812 LED control

// ==== Serial Number ====
#define USBTINY_CHANGE_SERIAL     55  // Change device serial number

// ==== Multi-byte transfer masks ====
// 0xE0-0xEF: I2C multi-byte (lower nibble = length, bit 3 = stop)
#define USBTINY_I2C_MULTI_MASK    0xE0
// 0xF0-0xFF: SPI multi-byte (lower nibble = length, bit 3 = auto CS)
#define USBTINY_SPI_MULTI_MASK    0xF0

// ==== WS2812 Control Bits ====
#define WS2812_ADD_TO_BUFFER      0x20  // Bit 5: Add to buffer
#define WS2812_FLUSH_BUFFER       0x10  // Bit 4: Send buffer

// ==== USB Request Type ====
// Little-Wire uses vendor-specific control transfers
// bmRequestType = 0xC0 (Device-to-Host, Vendor, Device)
#define LW_REQUEST_TYPE_IN        0xC0
#define LW_REQUEST_TYPE_OUT       0x40

#endif // _LITTLEWIRE_PROTOCOL_H
