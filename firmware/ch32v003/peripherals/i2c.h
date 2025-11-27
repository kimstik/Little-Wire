/**
 * @file i2c.h
 * @brief I2C driver header for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>

/**
 * @brief Initialize I2C (bitbang for compatibility)
 */
void i2c_init(void);

/**
 * @brief Send I2C start condition and address
 * @param address 7-bit address (shifted left by 1, LSB = R/W)
 * @return 0 if ACK received, 1 if NACK
 */
uint8_t i2c_start(uint8_t address);

/**
 * @brief Write byte to I2C bus
 * @param data Byte to write
 * @return 0 if ACK received, 1 if NACK
 */
uint8_t i2c_write(uint8_t data);

/**
 * @brief Read byte from I2C bus
 * @param ack 1 to send ACK, 0 to send NACK
 * @return Byte read
 */
uint8_t i2c_read(uint8_t ack);

/**
 * @brief Send I2C stop condition
 */
void i2c_stop(void);

/**
 * @brief Set I2C clock delay
 * @param delay Delay value (larger = slower clock)
 */
void i2c_set_delay(uint8_t delay);

#endif // _I2C_H
