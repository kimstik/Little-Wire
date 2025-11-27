/**
 * @file i2c.c
 * @brief Bitbang I2C driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 *
 * Uses bitbang I2C for maximum compatibility with original Little-Wire
 * behavior and timing. Hardware I2C can be added as an option later.
 */

#include "ch32fun.h"
#include "i2c.h"
#include "../pin_mapping.h"

// I2C pins (bitbang) - matching original ATtiny85 Little-Wire
// SCL = PC2 (PIN2) - matches PB2 on ATtiny85
// SDA = PC4 (PIN4) - matches PB0 on ATtiny85

#define I2C_SDA_PIN  4  // PC4 = PIN4 (was PB0 on ATtiny85)
#define I2C_SCL_PIN  2  // PC2 = PIN2 (was PB2 on ATtiny85)

static uint8_t i2c_delay = 5;

// Internal helpers
static inline void sda_high(void) {
    // Release SDA (input with pull-up = high)
    GPIOC->CFGLR &= ~(0xFUL << (I2C_SDA_PIN * 4));
    GPIOC->CFGLR |= (0x8UL << (I2C_SDA_PIN * 4));  // Input with pull
    GPIOC->BSHR = (1UL << I2C_SDA_PIN);  // Pull-up
}

static inline void sda_low(void) {
    // Drive SDA low
    GPIOC->CFGLR &= ~(0xFUL << (I2C_SDA_PIN * 4));
    GPIOC->CFGLR |= (0x1UL << (I2C_SDA_PIN * 4));  // Push-pull output
    GPIOC->BCR = (1UL << I2C_SDA_PIN);  // Low
}

static inline void scl_high(void) {
    // Release SCL (input with pull-up = high)
    GPIOC->CFGLR &= ~(0xFUL << (I2C_SCL_PIN * 4));
    GPIOC->CFGLR |= (0x8UL << (I2C_SCL_PIN * 4));  // Input with pull
    GPIOC->BSHR = (1UL << I2C_SCL_PIN);  // Pull-up
}

static inline void scl_low(void) {
    // Drive SCL low
    GPIOC->CFGLR &= ~(0xFUL << (I2C_SCL_PIN * 4));
    GPIOC->CFGLR |= (0x1UL << (I2C_SCL_PIN * 4));  // Push-pull output
    GPIOC->BCR = (1UL << I2C_SCL_PIN);  // Low
}

static inline uint8_t sda_read(void) {
    return (GPIOC->INDR >> I2C_SDA_PIN) & 1;
}

static inline void i2c_delay_us(void) {
    for (uint8_t i = 0; i < i2c_delay; i++) {
        Delay_Us(1);
    }
}

void i2c_init(void)
{
    // Enable GPIOC clock
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    // Initialize both lines high (released)
    // Output low initially, then switch to input for open-drain behavior
    GPIOC->BCR = (1 << 1) | (1 << 2);  // Both low when output

    scl_high();
    sda_high();

    i2c_delay_us();
}

void i2c_set_delay(uint8_t delay)
{
    i2c_delay = delay;
}

static void i2c_start_condition(void)
{
    // START: SDA goes low while SCL is high
    sda_high();
    scl_high();
    i2c_delay_us();

    sda_low();
    i2c_delay_us();

    scl_low();
    i2c_delay_us();
}

void i2c_stop(void)
{
    // STOP: SDA goes high while SCL is high
    sda_low();
    i2c_delay_us();

    scl_low();
    i2c_delay_us();

    scl_high();
    i2c_delay_us();

    sda_high();
    i2c_delay_us();
}

static void i2c_write_bit(uint8_t bit)
{
    if (bit) {
        sda_high();
    } else {
        sda_low();
    }
    i2c_delay_us();

    scl_high();
    i2c_delay_us();

    scl_low();
    i2c_delay_us();

    if (bit) {
        sda_low();
    }
    i2c_delay_us();
}

static uint8_t i2c_read_bit(void)
{
    uint8_t bit;

    sda_high();  // Release SDA for slave to drive
    i2c_delay_us();

    scl_high();
    i2c_delay_us();

    bit = sda_read();
    i2c_delay_us();

    scl_low();
    i2c_delay_us();

    return bit;
}

uint8_t i2c_start(uint8_t address)
{
    i2c_start_condition();
    return i2c_write(address);
}

uint8_t i2c_write(uint8_t data)
{
    // Write 8 bits, MSB first
    for (uint8_t i = 0; i < 8; i++) {
        i2c_write_bit(data & 0x80);
        data <<= 1;
    }

    // Read ACK (0 = ACK, 1 = NACK)
    return i2c_read_bit();
}

uint8_t i2c_read(uint8_t ack)
{
    uint8_t data = 0;

    // Read 8 bits, MSB first
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        data |= i2c_read_bit();
    }

    i2c_delay_us();

    // Send ACK/NACK
    if (ack) {
        i2c_write_bit(0);  // ACK
    } else {
        i2c_write_bit(1);  // NACK
    }

    i2c_delay_us();

    return data;
}
