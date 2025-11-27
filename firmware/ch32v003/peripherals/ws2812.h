/**
 * @file ws2812.h
 * @brief WS2812 LED driver header for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _WS2812_H
#define _WS2812_H

#include <stdint.h>

// Maximum number of LEDs in buffer
#define WS2812_MAX_LEDS 64

/**
 * @brief Preload RGB values into buffer
 * @param r Red value (0-255)
 * @param g Green value (0-255)
 * @param b Blue value (0-255)
 */
void ws2812_preload(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Flush buffer to WS2812 LEDs
 * @param pin GPIO pin number on Port C
 */
void ws2812_flush(uint8_t pin);

/**
 * @brief Reset buffer pointer
 */
void ws2812_reset(void);

/**
 * @brief Get current buffer position
 * @return Number of LEDs currently in buffer
 */
uint8_t ws2812_get_count(void);

#endif // _WS2812_H
