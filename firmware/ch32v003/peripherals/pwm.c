/**
 * @file pwm.c
 * @brief PWM driver using TIM1 for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32fun.h"
#include "pwm.h"
#include "../pin_mapping.h"

// Prescaler values to match ATtiny85 behavior
// ATtiny85 runs at 16.5MHz, CH32V003 at 48MHz
// Scale factor: 48/16.5 = 2.9, but we'll use approximate values
static const uint16_t prescaler_values[] = {
    3,      // 0: ~64kHz (was /1 on ATtiny85)
    24,     // 1: ~8kHz  (was /8 on ATtiny85)
    188,    // 2: ~1kHz  (was /64 on ATtiny85)
    750,    // 3: ~250Hz (was /256 on ATtiny85)
    2999    // 4: ~63Hz  (was /1024 on ATtiny85) - default
};

static uint8_t pwm_prescaler_index = 4;

void pwm_init(void)
{
    // Enable TIM1 clock
    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;

    // Enable GPIOC clock
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    // Configure PC1 (TIM1_CH1) as alternate function push-pull
    GPIOC->CFGLR &= ~(0xF << (1 * 4));
    GPIOC->CFGLR |= (0xB << (1 * 4));  // AF push-pull, 50MHz

    // Configure PC2 (TIM1_CH2) as alternate function push-pull
    GPIOC->CFGLR &= ~(0xF << (2 * 4));
    GPIOC->CFGLR |= (0xB << (2 * 4));  // AF push-pull, 50MHz

    // Configure TIM1 for 8-bit PWM
    TIM1->PSC = prescaler_values[pwm_prescaler_index];  // Prescaler
    TIM1->ATRLR = 255;  // Auto-reload for 8-bit resolution

    // Configure PWM mode 1 on CH1 (OC1M = 110, OC1PE = 1)
    TIM1->CHCTLR1 = (6 << 4) | (1 << 3);  // CH1: PWM mode 1, preload enable

    // Configure PWM mode 1 on CH2 (OC2M = 110, OC2PE = 1)
    TIM1->CHCTLR1 |= (6 << 12) | (1 << 11);  // CH2: PWM mode 1, preload enable

    // Enable CH1 and CH2 outputs
    TIM1->CCER = (1 << 0) | (1 << 4);  // CC1E, CC2E

    // Enable main output (required for TIM1)
    TIM1->BDTR = (1 << 15);  // MOE

    // Initialize compare values to 0
    TIM1->CH1CVR = 0;
    TIM1->CH2CVR = 0;

    // Enable counter
    TIM1->CTLR1 = (1 << 7) | (1 << 0);  // ARPE, CEN
}

void pwm_stop(void)
{
    // Disable timer
    TIM1->CTLR1 = 0;

    // Disable outputs
    TIM1->CCER = 0;
    TIM1->BDTR = 0;

    // Reset GPIO to regular output (low)
    GPIOC->CFGLR &= ~(0xF << (1 * 4));
    GPIOC->CFGLR |= (0x1 << (1 * 4));  // Push-pull output
    GPIOC->BCR = (1 << 1);  // Set low

    GPIOC->CFGLR &= ~(0xF << (2 * 4));
    GPIOC->CFGLR |= (0x1 << (2 * 4));  // Push-pull output
    GPIOC->BCR = (1 << 2);  // Set low
}

void pwm_set_compare(uint8_t ch_a, uint8_t ch_b)
{
    TIM1->CH1CVR = ch_a;
    TIM1->CH2CVR = ch_b;
}

void pwm_set_prescaler(uint8_t index)
{
    if (index > 4) index = 4;
    pwm_prescaler_index = index;
    TIM1->PSC = prescaler_values[index];
}
