/**
 * @file adc.h
 * @brief ADC driver header for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#ifndef _ADC_H
#define _ADC_H

#include <stdint.h>

// ADC voltage references (compatible with original Little-Wire)
#define VREF_VCC    0   // Use VCC as reference
#define VREF_1V1    1   // Internal 1.2V reference (CH32V003 has 1.2V)
#define VREF_2V56   2   // Not available on CH32V003, will use VCC

/**
 * @brief Initialize ADC peripheral
 * @param prescaler ADC clock prescaler (0-3)
 * @param reference Voltage reference selection
 */
void adc_init(uint8_t prescaler, uint8_t reference);

/**
 * @brief Read ADC channel
 * @param channel ADC channel number (0-1 for external, 8 for temperature)
 * @return 10-bit ADC value (0-1023)
 */
uint16_t adc_read(uint8_t channel);

#endif // _ADC_H
