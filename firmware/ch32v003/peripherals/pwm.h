/**
 * @file pwm.h
 * @brief PWM driver header for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _PWM_H
#define _PWM_H

#include <stdint.h>

/**
 * @brief Initialize PWM on TIM1 (CH1 and CH2)
 */
void pwm_init(void);

/**
 * @brief Stop PWM output
 */
void pwm_stop(void);

/**
 * @brief Update PWM compare values
 * @param ch_a Channel A (TIM1_CH1) compare value (0-255)
 * @param ch_b Channel B (TIM1_CH2) compare value (0-255)
 */
void pwm_set_compare(uint8_t ch_a, uint8_t ch_b);

/**
 * @brief Change PWM prescaler
 * @param index Prescaler index (0-4)
 *   0: /1   (~64kHz update frequency)
 *   1: /8   (~8kHz update frequency)
 *   2: /64  (~1kHz update frequency)
 *   3: /256 (~250Hz update frequency)
 *   4: /1024 (~63Hz update frequency) - default, suitable for servos
 */
void pwm_set_prescaler(uint8_t index);

#endif // _PWM_H
