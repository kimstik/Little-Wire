/**
 * @file onewire.c
 * @brief OneWire bitbang driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 *
 * Timing-critical OneWire implementation with interrupt protection
 */

#include "ch32fun.h"
#include "onewire.h"
#include "../pin_mapping.h"

// Default OneWire pin: PC4 (PIN4)
static uint8_t ow_pin = 4;

// Macros for pin control (using Port C)
#define OW_OUTPUT() do { \
    GPIOC->CFGLR &= ~(0xFUL << (ow_pin * 4)); \
    GPIOC->CFGLR |= (0x1UL << (ow_pin * 4)); \
} while(0)

#define OW_INPUT() do { \
    GPIOC->CFGLR &= ~(0xFUL << (ow_pin * 4)); \
    GPIOC->CFGLR |= (0x4UL << (ow_pin * 4)); \
} while(0)

#define OW_LOW()    (GPIOC->BCR = (1UL << ow_pin))
#define OW_HIGH()   (GPIOC->BSHR = (1UL << ow_pin))
#define OW_READ()   ((GPIOC->INDR >> ow_pin) & 1)

void onewire_set_pin(uint8_t pin)
{
    ow_pin = pin;
}

uint8_t onewire_reset(void)
{
    uint8_t presence;

    // Drive bus low for 480us
    OW_OUTPUT();
    OW_LOW();
    Delay_Us(480);

    // Release bus and wait 70us for presence pulse
    __disable_irq();
    OW_HIGH();
    Delay_Us(70);

    // Sample for presence (slave pulls low)
    OW_INPUT();
    presence = !OW_READ();
    __enable_irq();

    // Wait remainder of reset slot
    Delay_Us(410);

    // Release bus
    OW_OUTPUT();
    OW_HIGH();

    return presence;
}

void onewire_write_byte(uint8_t data)
{
    OW_OUTPUT();

    for (uint8_t i = 0; i < 8; i++) {
        __disable_irq();

        // Drive bus low
        OW_LOW();

        if (data & 0x01) {
            // Write '1': release bus after 6us
            Delay_Us(6);
            OW_HIGH();
            Delay_Us(64);
        } else {
            // Write '0': hold low for 60us
            Delay_Us(60);
            OW_HIGH();
            Delay_Us(10);
        }

        __enable_irq();

        data >>= 1;
    }
}

uint8_t onewire_read_byte(void)
{
    uint8_t data = 0;

    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;

        OW_OUTPUT();

        __disable_irq();

        // Initiate read slot: drive low for 6us
        OW_LOW();
        Delay_Us(6);

        // Release and wait for slave response
        OW_HIGH();
        Delay_Us(10);

        // Sample the bus
        OW_INPUT();
        if (OW_READ()) {
            data |= 0x80;
        }

        __enable_irq();

        // Wait for slot to complete
        Delay_Us(55);
    }

    return data;
}

void onewire_write_bit(uint8_t bit)
{
    OW_OUTPUT();

    __disable_irq();

    OW_LOW();

    if (bit & 0x01) {
        // Write '1'
        Delay_Us(6);
        OW_HIGH();
        Delay_Us(64);
    } else {
        // Write '0'
        Delay_Us(60);
        OW_HIGH();
        Delay_Us(10);
    }

    __enable_irq();
}

uint8_t onewire_read_bit(void)
{
    uint8_t bit;

    OW_OUTPUT();

    __disable_irq();

    // Initiate read slot
    OW_LOW();
    Delay_Us(6);

    OW_HIGH();
    Delay_Us(10);

    // Sample
    OW_INPUT();
    bit = OW_READ();

    __enable_irq();

    return bit;
}
