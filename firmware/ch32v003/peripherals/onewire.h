/**
 * @file onewire.h
 * @brief OneWire driver header for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _ONEWIRE_H
#define _ONEWIRE_H

#include <stdint.h>

/**
 * @brief Set OneWire data pin
 * @param pin GPIO pin number on Port C
 */
void onewire_set_pin(uint8_t pin);

/**
 * @brief Send OneWire reset pulse
 * @return 1 if presence pulse detected, 0 otherwise
 */
uint8_t onewire_reset(void);

/**
 * @brief Write byte to OneWire bus
 * @param data Byte to write
 */
void onewire_write_byte(uint8_t data);

/**
 * @brief Read byte from OneWire bus
 * @return Byte read
 */
uint8_t onewire_read_byte(void);

/**
 * @brief Write single bit to OneWire bus
 * @param bit Bit value (0 or 1)
 */
void onewire_write_bit(uint8_t bit);

/**
 * @brief Read single bit from OneWire bus
 * @return Bit value (0 or 1)
 */
uint8_t onewire_read_bit(void);

#endif // _ONEWIRE_H
