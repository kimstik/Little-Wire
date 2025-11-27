/**
 * @file spi.h
 * @brief SPI driver header for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

/**
 * @brief Initialize hardware SPI
 */
void spi_init(void);

/**
 * @brief Transfer single byte via SPI
 * @param data Byte to send
 * @return Byte received
 */
uint8_t spi_transfer(uint8_t data);

/**
 * @brief Transfer buffer via SPI
 * @param tx Transmit buffer
 * @param rx Receive buffer (can be same as tx)
 * @param len Number of bytes to transfer
 */
void spi_transfer_buffer(uint8_t* tx, uint8_t* rx, uint8_t len);

/**
 * @brief Set SPI delay between bytes
 * @param delay_us Delay in microseconds
 */
void spi_set_delay(uint16_t delay_us);

/**
 * @brief Debug SPI - slow bitbang mode (SPI mode 3)
 * @param data Byte to send
 * @return Byte received
 */
uint8_t spi_debug_transfer(uint8_t data);

#endif // _SPI_H
