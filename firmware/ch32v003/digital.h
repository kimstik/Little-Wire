/**
 * @file digital.h
 * @brief GPIO abstraction macros for CH32V003
 * @author kimstik
 * assisted by intelligence
 *
 * Provides Arduino-style GPIO macros compatible with original Little-Wire
 */

#ifndef _DIGITAL_H
#define _DIGITAL_H

#include "ch32fun.h"

// ==== Pin Modes ====
#define INPUT       0
#define OUTPUT      1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3

// ==== Pin States ====
#define LOW         0
#define HIGH        1
#define ENABLE      1
#define DISABLE     0

// ==== GPIO Port Base Addresses ====
#define GPIOA_ADDR  ((GPIO_TypeDef*)GPIOA_BASE)
#define GPIOC_ADDR  ((GPIO_TypeDef*)GPIOC_BASE)
#define GPIOD_ADDR  ((GPIO_TypeDef*)GPIOD_BASE)

// ==== Helper Macros ====
// Concatenate port letter to GPIO base
#define _GPIO_PORT_CONCAT(x) GPIO##x##_ADDR
#define GPIO_PORT(x) _GPIO_PORT_CONCAT(x)

// ==== GPIO Configuration ====
// CH32V003 uses CFGLR for pins 0-7

/**
 * @brief Configure pin mode
 * @param port Port letter (A, C, D)
 * @param pin Pin number (0-7)
 * @param mode INPUT, OUTPUT, INPUT_PULLUP, or INPUT_PULLDOWN
 */
#define pinMode(port, pin, mode) do { \
    GPIO_TypeDef* _gpio = GPIO_PORT(port); \
    uint32_t _cfg = _gpio->CFGLR; \
    _cfg &= ~(0xFUL << ((pin) * 4)); \
    if ((mode) == OUTPUT) { \
        _cfg |= (0x1UL << ((pin) * 4)); /* Push-pull, 10MHz */ \
    } else if ((mode) == INPUT_PULLUP) { \
        _cfg |= (0x8UL << ((pin) * 4)); /* Input with pull-up/down */ \
        _gpio->BSHR = (1UL << (pin)); /* Pull-up via ODR=1 */ \
    } else if ((mode) == INPUT_PULLDOWN) { \
        _cfg |= (0x8UL << ((pin) * 4)); /* Input with pull-up/down */ \
        _gpio->BCR = (1UL << (pin)); /* Pull-down via ODR=0 */ \
    } else { \
        _cfg |= (0x4UL << ((pin) * 4)); /* Floating input */ \
    } \
    _gpio->CFGLR = _cfg; \
} while(0)

/**
 * @brief Write digital value to pin
 * @param port Port letter (A, C, D)
 * @param pin Pin number (0-7)
 * @param state HIGH or LOW
 */
#define digitalWrite(port, pin, state) do { \
    GPIO_TypeDef* _gpio = GPIO_PORT(port); \
    if (state) { \
        _gpio->BSHR = (1UL << (pin)); /* Set bit */ \
    } else { \
        _gpio->BCR = (1UL << (pin)); /* Clear bit */ \
    } \
} while(0)

/**
 * @brief Read digital value from pin
 * @param port Port letter (A, C, D)
 * @param pin Pin number (0-7)
 * @return 0 or 1
 */
#define digitalRead(port, pin) \
    ((GPIO_PORT(port)->INDR >> (pin)) & 1UL)

/**
 * @brief Toggle pin state
 * @param port Port letter (A, C, D)
 * @param pin Pin number (0-7)
 */
#define togglePin(port, pin) do { \
    GPIO_TypeDef* _gpio = GPIO_PORT(port); \
    _gpio->OUTDR ^= (1UL << (pin)); \
} while(0)

/**
 * @brief Configure internal pullup
 * @param port Port letter (A, C, D)
 * @param pin Pin number (0-7)
 * @param state ENABLE or DISABLE
 *
 * Note: Pin must be configured as input first
 */
#define internalPullup(port, pin, state) do { \
    GPIO_TypeDef* _gpio = GPIO_PORT(port); \
    uint32_t _cfg = _gpio->CFGLR; \
    _cfg &= ~(0xFUL << ((pin) * 4)); \
    if (state) { \
        _cfg |= (0x8UL << ((pin) * 4)); /* Input with pull */ \
        _gpio->BSHR = (1UL << (pin)); /* Pull-up */ \
    } else { \
        _cfg |= (0x4UL << ((pin) * 4)); /* Floating input */ \
    } \
    _gpio->CFGLR = _cfg; \
} while(0)

// ==== Delay Macros ====
#define delayMicroseconds(us) Delay_Us(us)
#define delayMilliseconds(ms) Delay_Ms(ms)

// ==== Bit Manipulation Macros ====
#define sbi(reg, bit) ((reg) |= (1UL << (bit)))
#define cbi(reg, bit) ((reg) &= ~(1UL << (bit)))
#define checkBit(reg, bit) ((reg) & (1UL << (bit)))

#endif // _DIGITAL_H
