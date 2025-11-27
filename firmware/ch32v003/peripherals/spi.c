/**
 * @file spi.c
 * @brief SPI driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32fun.h"
#include "spi.h"
#include "../pin_mapping.h"
#include "../digital.h"

static uint16_t spi_delay_us = 0;
static uint8_t spi_initialized = 0;

void spi_init(void)
{
    // Enable SPI1 clock
    RCC->APB2PCENR |= RCC_APB2Periph_SPI1;

    // Enable GPIOC clock
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    // Configure GPIO:
    // PC5 = SCK  (AF push-pull)
    // PC6 = MOSI (AF push-pull)
    // PC7 = MISO (floating input)

    // SCK - PC5
    GPIOC->CFGLR &= ~(0xF << (5 * 4));
    GPIOC->CFGLR |= (0xB << (5 * 4));  // AF push-pull, 50MHz

    // MOSI - PC6
    GPIOC->CFGLR &= ~(0xF << (6 * 4));
    GPIOC->CFGLR |= (0xB << (6 * 4));  // AF push-pull, 50MHz

    // MISO - PC7
    GPIOC->CFGLR &= ~(0xF << (7 * 4));
    GPIOC->CFGLR |= (0x4 << (7 * 4));  // Floating input

    // Reset SPI
    RCC->APB2PRSTR |= RCC_APB2Periph_SPI1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_SPI1;

    // Configure SPI1:
    // - Master mode
    // - CPOL=0, CPHA=0 (SPI mode 0)
    // - MSB first
    // - Software slave management
    // - Baud rate: /16 (~3MHz at 48MHz)
    SPI1->CTLR1 = (1 << 9)  |  // SSM: Software slave management
                  (1 << 8)  |  // SSI: Internal slave select high
                  (3 << 3)  |  // BR: /16 baud rate
                  (1 << 2);    // MSTR: Master mode

    // Enable SPI
    SPI1->CTLR1 |= (1 << 6);  // SPE

    spi_initialized = 1;
}

uint8_t spi_transfer(uint8_t data)
{
    if (!spi_initialized) {
        spi_init();
    }

    // Wait for TX buffer empty
    while(!(SPI1->STATR & (1 << 1)));  // TXE

    // Send data
    SPI1->DATAR = data;

    // Wait for RX buffer not empty
    while(!(SPI1->STATR & (1 << 0)));  // RXNE

    // Return received data
    return (uint8_t)SPI1->DATAR;
}

void spi_transfer_buffer(uint8_t* tx, uint8_t* rx, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        rx[i] = spi_transfer(tx[i]);
        if (spi_delay_us > 0) {
            Delay_Us(spi_delay_us);
        }
    }
}

void spi_set_delay(uint16_t delay_us)
{
    spi_delay_us = delay_us;
}

// Debug SPI - bitbang mode (SPI mode 3: CPOL=1, CPHA=1)
// Slower but more compatible with various devices
uint8_t spi_debug_transfer(uint8_t data)
{
    // Use bitbang on same pins for debug mode
    // SCK idle high (mode 3)

    uint8_t result = 0;

    // Configure pins for bitbang
    // SCK - output, start high
    GPIOC->CFGLR &= ~(0xF << (5 * 4));
    GPIOC->CFGLR |= (0x1 << (5 * 4));  // Push-pull output
    GPIOC->BSHR = (1 << 5);  // SCK high (idle)

    // MOSI - output
    GPIOC->CFGLR &= ~(0xF << (6 * 4));
    GPIOC->CFGLR |= (0x1 << (6 * 4));  // Push-pull output

    // MISO - input
    GPIOC->CFGLR &= ~(0xF << (7 * 4));
    GPIOC->CFGLR |= (0x4 << (7 * 4));  // Floating input

    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        // Set MOSI
        if (data & mask) {
            GPIOC->BSHR = (1 << 6);  // MOSI high
        } else {
            GPIOC->BCR = (1 << 6);   // MOSI low
        }

        if (spi_delay_us > 0) Delay_Us(spi_delay_us);

        // Clock low (capture on falling edge for mode 3)
        GPIOC->BCR = (1 << 5);  // SCK low

        // Shift result
        result <<= 1;

        // Sample MISO
        if (GPIOC->INDR & (1 << 7)) {
            result |= 1;
        }

        if (spi_delay_us > 0) Delay_Us(spi_delay_us);

        // Clock high
        GPIOC->BSHR = (1 << 5);  // SCK high
    }

    // Clear MOSI
    GPIOC->BCR = (1 << 6);

    return result;
}
