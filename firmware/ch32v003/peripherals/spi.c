/**
 * @file spi.c
 * @brief Bitbang SPI driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 *
 * Uses bitbang SPI for compatibility with original Little-Wire pin mapping.
 * Original ATtiny85 uses USI in SPI mode on:
 *   SCK  = PB2 (PIN2)
 *   MOSI = PB0 (PIN4)
 *   MISO = PB1 (PIN1)
 *
 * CH32V003 mapping:
 *   SCK  = PC2 (PIN2)
 *   MOSI = PC4 (PIN4)
 *   MISO = PC1 (PIN1)
 */

#include "ch32fun.h"
#include "spi.h"
#include "../pin_mapping.h"

// SPI pins - matching original ATtiny85 Little-Wire
#define SPI_SCK_PIN   2  // PC2 = PIN2 (was PB2 on ATtiny85)
#define SPI_MOSI_PIN  4  // PC4 = PIN4 (was PB0 on ATtiny85)
#define SPI_MISO_PIN  1  // PC1 = PIN1 (was PB1 on ATtiny85)

static uint16_t spi_delay_us = 0;
static uint8_t spi_initialized = 0;

// Pin control macros
#define SCK_HIGH()   (GPIOC->BSHR = (1UL << SPI_SCK_PIN))
#define SCK_LOW()    (GPIOC->BCR = (1UL << SPI_SCK_PIN))
#define MOSI_HIGH()  (GPIOC->BSHR = (1UL << SPI_MOSI_PIN))
#define MOSI_LOW()   (GPIOC->BCR = (1UL << SPI_MOSI_PIN))
#define MISO_READ()  ((GPIOC->INDR >> SPI_MISO_PIN) & 1)

void spi_init(void)
{
    // Enable GPIOC clock
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    // Configure SCK as push-pull output, start low (mode 0)
    GPIOC->CFGLR &= ~(0xFUL << (SPI_SCK_PIN * 4));
    GPIOC->CFGLR |= (0x3UL << (SPI_SCK_PIN * 4));  // Push-pull, 50MHz
    SCK_LOW();

    // Configure MOSI as push-pull output
    GPIOC->CFGLR &= ~(0xFUL << (SPI_MOSI_PIN * 4));
    GPIOC->CFGLR |= (0x3UL << (SPI_MOSI_PIN * 4));  // Push-pull, 50MHz
    MOSI_LOW();

    // Configure MISO as floating input
    GPIOC->CFGLR &= ~(0xFUL << (SPI_MISO_PIN * 4));
    GPIOC->CFGLR |= (0x4UL << (SPI_MISO_PIN * 4));  // Floating input

    spi_initialized = 1;
}

// SPI Mode 0: CPOL=0, CPHA=0
// Clock idles low, data sampled on rising edge
uint8_t spi_transfer(uint8_t data)
{
    uint8_t result = 0;

    if (!spi_initialized) {
        spi_init();
    }

    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        // Set MOSI before clock rise
        if (data & mask) {
            MOSI_HIGH();
        } else {
            MOSI_LOW();
        }

        if (spi_delay_us > 0) Delay_Us(spi_delay_us);

        // Rising edge - data is sampled
        SCK_HIGH();

        // Shift result and sample MISO
        result <<= 1;
        if (MISO_READ()) {
            result |= 1;
        }

        if (spi_delay_us > 0) Delay_Us(spi_delay_us);

        // Falling edge
        SCK_LOW();
    }

    // Leave MOSI low
    MOSI_LOW();

    return result;
}

void spi_transfer_buffer(uint8_t* tx, uint8_t* rx, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        rx[i] = spi_transfer(tx[i]);
    }
}

void spi_set_delay(uint16_t delay_us)
{
    spi_delay_us = delay_us;
}

// Debug SPI - Mode 3: CPOL=1, CPHA=1
// Clock idles high, data sampled on falling edge
// This matches the original Little-Wire debugSPI mode
uint8_t spi_debug_transfer(uint8_t data)
{
    uint8_t result = 0;

    if (!spi_initialized) {
        spi_init();
    }

    // Set clock high for mode 3 idle
    SCK_HIGH();

    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        // Set MOSI
        if (data & mask) {
            MOSI_HIGH();
        } else {
            MOSI_LOW();
        }

        if (spi_delay_us > 0) Delay_Us(spi_delay_us);

        // Falling edge - data is sampled in mode 3
        SCK_LOW();

        // Shift result and sample MISO
        result <<= 1;
        if (MISO_READ()) {
            result |= 1;
        }

        if (spi_delay_us > 0) Delay_Us(spi_delay_us);

        // Rising edge
        SCK_HIGH();
    }

    // Leave MOSI low, SCK high (mode 3 idle)
    MOSI_LOW();

    return result;
}
