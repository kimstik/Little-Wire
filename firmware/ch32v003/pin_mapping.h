/**
 * @file pin_mapping.h
 * @brief Little-Wire pin mapping for CH32V003
 * @author kimstik
 * assisted by intelligence
 *
 * Maps Little-Wire logical pins to CH32V003 physical GPIO
 * Using TSSOP20 package for full GPIO availability
 */

#ifndef _PIN_MAPPING_H
#define _PIN_MAPPING_H

// ==== Little-Wire Logical Pin Definitions ====
// These match the original ATtiny85 Little-Wire pin numbering

// PIN1 - General purpose IO / PWM capable
#define PIN1_PORT   C
#define PIN1_PIN    1

// PIN2 - General purpose IO / SCK / I2C SCL
#define PIN2_PORT   C
#define PIN2_PIN    2

// PIN3 - General purpose IO (was RESET on ATtiny85)
#define PIN3_PORT   C
#define PIN3_PIN    3

// PIN4 - General purpose IO / PWM capable / I2C SDA
#define PIN4_PORT   C
#define PIN4_PIN    4

// ==== CH32V003 Bonus Pins (not on ATtiny85) ====
#define PIN5_PORT   C
#define PIN5_PIN    5

#define PIN6_PORT   C
#define PIN6_PIN    6

#define PIN7_PORT   C
#define PIN7_PIN    7

// ==== USB Pins (Fixed - Do not change) ====
#define USB_DP_PORT  D
#define USB_DP_PIN   3

#define USB_DM_PORT  D
#define USB_DM_PIN   4

#define USB_DPU_PORT D
#define USB_DPU_PIN  5

// ==== SPI Peripheral Mapping ====
// Using bitbang SPI on Little-Wire compatible pins
// Matches original ATtiny85: SCK=PB2, MOSI=PB0, MISO=PB1
#define SPI_SCK_PORT  C
#define SPI_SCK_PIN   2  // PIN2 (was PB2 on ATtiny85)

#define SPI_MOSI_PORT C
#define SPI_MOSI_PIN  4  // PIN4 (was PB0 on ATtiny85)

#define SPI_MISO_PORT C
#define SPI_MISO_PIN  1  // PIN1 (was PB1 on ATtiny85)

#define SPI_CS_PORT   C
#define SPI_CS_PIN    3  // Default chip select (PIN3)

// ==== I2C Peripheral Mapping ====
// Using bitbang I2C on Little-Wire compatible pins
// Matches original ATtiny85: SCL=PB2, SDA=PB0
#define I2C_SCL_PORT  C
#define I2C_SCL_PIN   2  // PIN2 (was PB2 on ATtiny85)

#define I2C_SDA_PORT  C
#define I2C_SDA_PIN   4  // PIN4 (was PB0 on ATtiny85)

// ==== ADC Channel Mapping ====
// CH32V003 has ADC on PA1, PA2, PC4, PD2-PD6
#define ADC_CH0_PORT  A
#define ADC_CH0_PIN   2
#define ADC_CH0_CHANNEL 0  // ADC channel number

#define ADC_CH1_PORT  A
#define ADC_CH1_PIN   1
#define ADC_CH1_CHANNEL 1

// Internal temperature sensor
#define ADC_TEMP_CHANNEL 8

// ==== PWM Mapping ====
// Using TIM1 on CH32V003
#define PWM_CH1_PORT  C   // TIM1_CH1 -> PC1 (PIN1)
#define PWM_CH1_PIN   1

#define PWM_CH2_PORT  C   // TIM1_CH2 -> PC2 (PIN2)
#define PWM_CH2_PIN   2

// ==== OneWire Default Pin ====
// Matches original ATtiny85: DATA_PIN=PB2 (PIN2)
#define ONEWIRE_PORT  C
#define ONEWIRE_PIN   2   // PIN2 by default (was PB2 on ATtiny85)

// ==== WS2812 Default Pin ====
#define WS2812_PORT   C
#define WS2812_PIN    1   // PIN1 by default

// ==== Debug Pin (optional) ====
#define DEBUG_PORT    D
#define DEBUG_PIN     0

// ==== Pin Number to Port/Pin Translation ====
// Helper macros to convert Little-Wire pin number to port/pin

// Get port letter from pin number (0-7)
static inline char lw_pin_to_port(uint8_t pin) {
    switch(pin) {
        case 0: return 'C';  // PIN4 -> PC4
        case 1: return 'C';  // PIN1 -> PC1
        case 2: return 'C';  // PIN2 -> PC2
        case 5: return 'C';  // PIN3 -> PC3
        default: return 'C';
    }
}

// Get GPIO pin number from Little-Wire pin number
static inline uint8_t lw_pin_to_gpio(uint8_t pin) {
    // Little-Wire pin mapping:
    // pin 0 -> PC4 (PIN4)
    // pin 1 -> PC1 (PIN1)
    // pin 2 -> PC2 (PIN2)
    // pin 5 -> PC3 (PIN3)
    switch(pin) {
        case 0: return 4;  // PB0 on ATtiny85 -> PC4
        case 1: return 1;  // PB1 on ATtiny85 -> PC1
        case 2: return 2;  // PB2 on ATtiny85 -> PC2
        case 5: return 3;  // PB5 on ATtiny85 -> PC3
        default: return pin;
    }
}

// ==== Package Pin Mapping (TSSOP20) ====
/*
 * CH32V003F4P6 (TSSOP20) Pin Mapping:
 *
 * Pin 1:  PD4 / USB D-
 * Pin 2:  PD5 / USB DPU (Pull-up control)
 * Pin 3:  PD6 / GPIO
 * Pin 4:  PD7 / NRST
 * Pin 5:  PA1 / ADC1
 * Pin 6:  PA2 / ADC0
 * Pin 7:  VSS (GND)
 * Pin 8:  PD0 / GPIO
 * Pin 9:  PD1 / GPIO (SWIO)
 * Pin 10: PC0 / GPIO
 * Pin 11: PC1 / GPIO / TIM1_CH1 / SPI_MISO    -> Little-Wire PIN1
 * Pin 12: PC2 / GPIO / TIM1_CH2 / SCK/SCL/OW  -> Little-Wire PIN2
 * Pin 13: PC3 / GPIO / TIM1_CH3               -> Little-Wire PIN3
 * Pin 14: PC4 / GPIO / TIM1_CH4 / MOSI/SDA    -> Little-Wire PIN4
 * Pin 15: PC5 / GPIO (available for expansion)
 * Pin 16: PC6 / GPIO (available for expansion)
 * Pin 17: PC7 / GPIO (available for expansion)
 * Pin 18: PD2 / GPIO / USB D+ (alternate)
 * Pin 19: PD3 / GPIO / USB D+
 * Pin 20: VDD (3.3V)
 */

#endif // _PIN_MAPPING_H
