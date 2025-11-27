/**
 * @file adc.c
 * @brief ADC driver for CH32V003 Little-Wire
 * @author kimstik
 * assisted by intelligence
 */

#include "ch32fun.h"
#include "adc.h"

static uint8_t adc_initialized = 0;

void adc_init(uint8_t prescaler, uint8_t reference)
{
    (void)reference; // CH32V003 uses internal reference

    // Enable ADC clock
    RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

    // Reset ADC
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Configure ADC clock prescaler
    // ADCPRE in RCC_CFGR0: 00=/2, 01=/4, 10=/6, 11=/8
    RCC->CFGR0 &= ~(3 << 14);
    RCC->CFGR0 |= ((prescaler & 0x03) << 14);

    // Configure ADC: software trigger, single conversion
    ADC1->CTLR2 = ADC_ADON | ADC_EXTSEL;  // Enable ADC, software trigger

    // Configure sample time for all channels (239.5 cycles for accuracy)
    ADC1->SAMPTR2 = 0x3FFFFFFF;  // All channels: 239.5 cycles

    // Wait for ADC to stabilize
    Delay_Us(10);

    // Perform calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while(ADC1->CTLR2 & ADC_RSTCAL);

    ADC1->CTLR2 |= ADC_CAL;
    while(ADC1->CTLR2 & ADC_CAL);

    adc_initialized = 1;
}

uint16_t adc_read(uint8_t channel)
{
    if (!adc_initialized) {
        adc_init(2, 0);  // Default: /6 prescaler, VCC reference
    }

    // Configure GPIO for analog input if needed
    if (channel == 0) {
        // PA2 = ADC channel 0
        GPIOA->CFGLR &= ~(0xF << (2 * 4));  // Analog input (CNF=0, MODE=0)
    } else if (channel == 1) {
        // PA1 = ADC channel 1
        GPIOA->CFGLR &= ~(0xF << (1 * 4));  // Analog input
    }
    // Channel 8 is internal temperature sensor, no GPIO needed

    // Select channel
    ADC1->RSQR3 = channel & 0x0F;
    ADC1->RSQR1 = 0;  // 1 conversion

    // Start conversion
    ADC1->CTLR2 |= ADC_SWSTART;

    // Wait for conversion to complete
    while(!(ADC1->STATR & ADC_EOC));

    // Read and return 10-bit result
    return (uint16_t)(ADC1->RDATAR & 0x3FF);
}
