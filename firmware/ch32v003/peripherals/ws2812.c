/**
 * @file ws2812.c
 * @brief WS2812 LED bitbang driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 *
 * Optimized 800kHz bitbang implementation for WS2812B LEDs
 * Timing at 48MHz:
 *   T0H = 0.35us = ~17 cycles
 *   T0L = 0.80us = ~38 cycles
 *   T1H = 0.70us = ~34 cycles
 *   T1L = 0.60us = ~29 cycles
 */

#include "ch32fun.h"
#include "ws2812.h"

// GRB buffer (WS2812 uses GRB order)
static uint8_t ws2812_buffer[WS2812_MAX_LEDS * 3];
static uint8_t ws2812_ptr = 0;

void ws2812_preload(uint8_t r, uint8_t g, uint8_t b)
{
    if (ws2812_ptr < WS2812_MAX_LEDS * 3) {
        ws2812_buffer[ws2812_ptr++] = g;  // GRB order
        ws2812_buffer[ws2812_ptr++] = r;
        ws2812_buffer[ws2812_ptr++] = b;
    }
}

void ws2812_reset(void)
{
    ws2812_ptr = 0;
}

uint8_t ws2812_get_count(void)
{
    return ws2812_ptr / 3;
}

void ws2812_flush(uint8_t pin)
{
    if (ws2812_ptr == 0) return;

    // Configure pin as output
    GPIOC->CFGLR &= ~(0xFUL << (pin * 4));
    GPIOC->CFGLR |= (0x3UL << (pin * 4));  // Push-pull, 50MHz

    uint32_t mask = (1UL << pin);

    __disable_irq();

    for (uint16_t i = 0; i < ws2812_ptr; i++) {
        uint8_t byte = ws2812_buffer[i];

        for (uint8_t bit = 0x80; bit; bit >>= 1) {
            if (byte & bit) {
                // Send '1': high ~0.7us, low ~0.6us
                GPIOC->BSHR = mask;  // Set high
                // ~34 cycles at 48MHz
                __asm__ volatile(
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n"
                );
                GPIOC->BCR = mask;   // Set low
                // ~29 cycles
                __asm__ volatile(
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                );
            } else {
                // Send '0': high ~0.35us, low ~0.8us
                GPIOC->BSHR = mask;  // Set high
                // ~17 cycles at 48MHz
                __asm__ volatile(
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n"
                );
                GPIOC->BCR = mask;   // Set low
                // ~38 cycles
                __asm__ volatile(
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                    "nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n"
                );
            }
        }
    }

    __enable_irq();

    // Reset buffer for next use
    ws2812_ptr = 0;
}
