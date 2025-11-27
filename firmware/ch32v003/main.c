/**
 * @file main.c
 * @brief Little-Wire CH32V003 firmware
 * @author kimstik
 * assisted by intelligence
 *
 * Little-Wire USB interface for CH32V003 microcontroller.
 * Protocol compatible with original Little-Wire (ATtiny85) firmware v1.3.
 *
 * Based on:
 * - Little-Wire by ihsan Kehribar and chris chung
 * - USB implementation via rv003usb by CNLohr
 *
 * License: GPL v2 (same as original Little-Wire)
 */

#define INSTANCE_DESCRIPTORS

#include "ch32fun.h"
#include "rv003usb.h"
#include "littlewire_protocol.h"
#include "digital.h"
#include "pin_mapping.h"

// Peripheral drivers
#include "peripherals/adc.h"
#include "peripherals/pwm.h"
#include "peripherals/spi.h"
#include "peripherals/i2c.h"
#include "peripherals/onewire.h"
#include "peripherals/ws2812.h"

// ==== Global State ====
static volatile uint8_t sendBuffer[9];
static volatile uint8_t rxBuffer[8];
static volatile uint8_t jobState = 0;

// SPI state
static uint16_t spi_delay = 0;
static uint8_t sck_period = 10;

// Soft PWM state
static uint8_t softPWM = 0;
static uint8_t compare0 = 0, compare1 = 0, compare2 = 0;
static uint8_t cmp0 = 0, cmp1 = 0, cmp2 = 0;
static uint8_t counter = 0;

// ==== Pin Control Functions ====

static void set_pin_input(uint8_t pin)
{
    uint8_t gpio_pin = lw_pin_to_gpio(pin);
    GPIOC->CFGLR &= ~(0xFUL << (gpio_pin * 4));
    GPIOC->CFGLR |= (0x4UL << (gpio_pin * 4));  // Floating input
}

static void set_pin_output(uint8_t pin)
{
    uint8_t gpio_pin = lw_pin_to_gpio(pin);
    GPIOC->CFGLR &= ~(0xFUL << (gpio_pin * 4));
    GPIOC->CFGLR |= (0x1UL << (gpio_pin * 4));  // Push-pull output
}

static void set_pin_high(uint8_t pin)
{
    uint8_t gpio_pin = lw_pin_to_gpio(pin);
    GPIOC->BSHR = (1UL << gpio_pin);
}

static void set_pin_low(uint8_t pin)
{
    uint8_t gpio_pin = lw_pin_to_gpio(pin);
    GPIOC->BCR = (1UL << gpio_pin);
}

static uint8_t read_pin(uint8_t pin)
{
    uint8_t gpio_pin = lw_pin_to_gpio(pin);
    return (GPIOC->INDR >> gpio_pin) & 1;
}

static void power_down_pins(void)
{
    // Set all Little-Wire pins to floating input
    for (uint8_t i = 1; i <= 4; i++) {
        uint8_t gpio_pin = lw_pin_to_gpio(i == 4 ? 0 : i);
        GPIOC->CFGLR &= ~(0xFUL << (gpio_pin * 4));
        GPIOC->CFGLR |= (0x4UL << (gpio_pin * 4));  // Floating input
    }
}

// ==== USB Control Message Handler ====

void usb_handle_other_control_message(struct usb_endpoint * e, struct usb_urb * s, struct rv003usb_internal * ist)
{
    (void)ist;

    // Extract request parameters
    uint8_t req = (s->wRequestTypeLSBRequestMSB >> 8) & 0xFF;
    uint8_t wValueL = s->lValueLSBIndexMSB & 0xFF;
    uint8_t wValueH = (s->lValueLSBIndexMSB >> 8) & 0xFF;
    uint8_t wIndexL = (s->lValueLSBIndexMSB >> 16) & 0xFF;
    uint8_t wIndexH = (s->lValueLSBIndexMSB >> 24) & 0xFF;
    uint8_t bit = wValueL & 7;

    // Initialize response
    sendBuffer[8] = 0;  // Default: no data to send

    // Handle Little-Wire protocol
    switch(req) {
        case USBTINY_ECHO:  // 0
            sendBuffer[0] = wValueL;
            sendBuffer[1] = 0x21;  // Echo signature
            e->opaque = (uint8_t*)sendBuffer;
            e->max_len = 8;
            break;

        case USBTINY_READ:  // 1 - Read port
            sendBuffer[0] = GPIOC->INDR & 0xFF;
            e->opaque = (uint8_t*)sendBuffer;
            e->max_len = 1;
            break;

        case USBTINY_WRITE:  // 2 - Write port
            GPIOC->OUTDR = wValueL;
            usb_send_empty(0);
            return;

        case USBTINY_CLR:  // 3 - Clear bit
            GPIOC->BCR = (1 << bit);
            usb_send_empty(0);
            return;

        case USBTINY_SET:  // 4 - Set bit
            GPIOC->BSHR = (1 << bit);
            usb_send_empty(0);
            return;

        case USBTINY_POWERUP:  // 5
            sck_period = wValueL;
            // Configure SPI pins for ISP mode
            spi_init();
            usb_send_empty(0);
            return;

        case USBTINY_POWERDOWN:  // 6
            power_down_pins();
            usb_send_empty(0);
            return;

        case USBTINY_SPI:  // 7 - 4-byte SPI command
            {
                uint8_t cmd[4], res[4];
                cmd[0] = wValueL;
                cmd[1] = wValueH;
                cmd[2] = wIndexL;
                cmd[3] = wIndexH;
                for (int i = 0; i < 4; i++) {
                    res[i] = spi_transfer(cmd[i]);
                }
                sendBuffer[0] = res[0];
                sendBuffer[1] = res[1];
                sendBuffer[2] = res[2];
                sendBuffer[3] = res[3];
                e->opaque = (uint8_t*)sendBuffer;
                e->max_len = 4;
            }
            break;

        case USBTINY_PIN_SET_INPUT:  // 13
            set_pin_input(bit);
            usb_send_empty(0);
            return;

        case USBTINY_PIN_SET_OUTPUT:  // 14
            set_pin_output(bit);
            usb_send_empty(0);
            return;

        case USBTINY_READ_ADC:  // 15
            {
                uint16_t adc_val;
                if (wValueL == 2) {
                    // Temperature sensor
                    adc_val = adc_read(8);
                } else {
                    adc_val = adc_read(wValueL);
                }
                sendBuffer[0] = adc_val & 0xFF;
                sendBuffer[1] = (adc_val >> 8) & 0xFF;
                e->opaque = (uint8_t*)sendBuffer;
                e->max_len = 2;
            }
            break;

        case USBTINY_SETUP_PWM:  // 16
            pwm_init();
            usb_send_empty(0);
            return;

        case USBTINY_UPDATE_PWM_COMPARE:  // 17
            pwm_set_compare(wValueL, wIndexL);
            usb_send_empty(0);
            return;

        case USBTINY_PIN_SET_HIGH:  // 18
            set_pin_high(bit);
            usb_send_empty(0);
            return;

        case USBTINY_PIN_SET_LOW:  // 19
            set_pin_low(bit);
            usb_send_empty(0);
            return;

        case USBTINY_PIN_READ:  // 20
            sendBuffer[0] = read_pin(bit);
            e->opaque = (uint8_t*)sendBuffer;
            e->max_len = 1;
            break;

        case USBTINY_SINGLE_SPI:  // 21
            sendBuffer[0] = spi_transfer(wValueL);
            e->opaque = (uint8_t*)sendBuffer;
            e->max_len = 1;
            break;

        case USBTINY_CHANGE_PWM_PRESCALE:  // 22
            pwm_set_prescaler(wValueL);
            usb_send_empty(0);
            return;

        case USBTINY_SETUP_SPI:  // 23
            spi_init();
            usb_send_empty(0);
            return;

        case USBTINY_SETUP_I2C:  // 24
            i2c_init();
            usb_send_empty(0);
            return;

        case USBTINY_I2C_BEGIN_TX:  // 25
            {
                uint8_t ack = i2c_start(wValueL);
                sendBuffer[0] = ack;
                sendBuffer[8] = 1;
                e->opaque = (uint8_t*)sendBuffer;
                e->max_len = 1;
            }
            break;

        case USBTINY_I2C_REQUEST_FROM:  // 30
            // This is handled in jobState processing
            jobState = 11;
            rxBuffer[0] = wValueL;  // End with NACK?
            rxBuffer[1] = wValueH;  // Length
            rxBuffer[2] = wIndexL;  // Issue stop?
            usb_send_empty(0);
            return;

        case USBTINY_SPI_UPDATE_DELAY:  // 31
            spi_delay = (wValueH << 8) | wValueL;
            spi_set_delay(spi_delay);
            usb_send_empty(0);
            return;

        case USBTINY_STOP_PWM:  // 32
            pwm_stop();
            usb_send_empty(0);
            return;

        case USBTINY_DEBUG_SPI:  // 33
            rxBuffer[0] = wValueL;
            jobState = 1;
            usb_send_empty(0);
            return;

        case USBTINY_VERSION_QUERY:  // 34
            sendBuffer[0] = LITTLE_WIRE_VERSION;
            e->opaque = (uint8_t*)sendBuffer;
            e->max_len = 1;
            break;

        case USBTINY_INIT_ADC:  // 35
            adc_init(wValueL & 0x07, wValueH);
            usb_send_empty(0);
            return;

        case USBTINY_READ_BUFFER:  // 40
            e->opaque = (uint8_t*)sendBuffer;
            e->max_len = sendBuffer[8];
            break;

        case USBTINY_ONEWIRE_RESET:  // 41
            jobState = 2;
            usb_send_empty(0);
            return;

        case USBTINY_ONEWIRE_SEND_BYTE:  // 42
            rxBuffer[0] = wValueL;
            jobState = 3;
            usb_send_empty(0);
            return;

        case USBTINY_ONEWIRE_READ_BYTE:  // 43
            jobState = 4;
            usb_send_empty(0);
            return;

        case 44:  // I2C init (alternative)
            i2c_init();
            jobState = 8;
            usb_send_empty(0);
            return;

        case 45:  // I2C begin (alternative)
            rxBuffer[0] = wValueL;
            jobState = 9;
            usb_send_empty(0);
            return;

        case 46:  // I2C read
            rxBuffer[0] = wValueL;  // End with NACK?
            rxBuffer[1] = wValueH;  // Length
            rxBuffer[2] = wIndexL;  // Issue stop?
            jobState = 11;
            usb_send_empty(0);
            return;

        case USBTINY_SOFTPWM_INIT:  // 47
            if (wValueL) {
                pinMode(C, 1, OUTPUT);
                pinMode(C, 2, OUTPUT);
                pinMode(C, 4, OUTPUT);
                softPWM = 1;
            } else {
                softPWM = 0;
            }
            usb_send_empty(0);
            return;

        case USBTINY_SOFTPWM_UPDATE:  // 48
            compare0 = wValueL;
            compare1 = wValueH;
            compare2 = wIndexL;
            usb_send_empty(0);
            return;

        case USBTINY_I2C_UPDATE_DELAY:  // 49
            i2c_set_delay(wValueL);
            usb_send_empty(0);
            return;

        case USBTINY_ONEWIRE_READ_BIT:  // 50
            jobState = 5;
            usb_send_empty(0);
            return;

        case USBTINY_ONEWIRE_WRITE_BIT:  // 51
            rxBuffer[0] = wValueL;
            jobState = 6;
            usb_send_empty(0);
            return;

        case USBTINY_WS2812_WRITE:  // 54
            {
                if (wValueL & WS2812_ADD_TO_BUFFER) {
                    // Add RGB to buffer (wValueH=G, wIndexL=R, wIndexH=B)
                    ws2812_preload(wIndexL, wValueH, wIndexH);
                }
                if (wValueL & WS2812_FLUSH_BUFFER) {
                    // Flush buffer to LEDs
                    uint8_t pin = lw_pin_to_gpio(bit);
                    set_pin_output(bit);
                    jobState = 17;
                    rxBuffer[0] = pin;
                }
            }
            usb_send_empty(0);
            return;

        case USBTINY_CHANGE_SERIAL:  // 55
            // Not implemented - CH32V003 doesn't have EEPROM
            // Could use flash, but skip for now
            usb_send_empty(0);
            return;

        default:
            // Handle multi-byte I2C (0xE0-0xEF)
            if ((req & 0xF0) == USBTINY_I2C_MULTI_MASK) {
                jobState = 10;
                rxBuffer[0] = req & 0x07;  // Length
                rxBuffer[1] = req & 0x08;  // Send stop?
                rxBuffer[2] = wValueL;
                rxBuffer[3] = wValueH;
                rxBuffer[4] = wIndexL;
                rxBuffer[5] = wIndexH;
                usb_send_empty(0);
                return;
            }

            // Handle multi-byte SPI (0xF0-0xFF)
            if ((req & 0xF0) == USBTINY_SPI_MULTI_MASK) {
                jobState = 7;
                rxBuffer[0] = req & 0x08;  // Auto chip select?
                rxBuffer[1] = req & 0x07;  // Length
                rxBuffer[2] = wValueL;
                rxBuffer[3] = wValueH;
                rxBuffer[4] = wIndexL;
                rxBuffer[5] = wIndexH;
                usb_send_empty(0);
                return;
            }

            usb_send_empty(0);
            return;
    }

    // Send response if data was prepared
    if (e->max_len > 0 && e->opaque) {
        usb_send_data(e->opaque, e->max_len, 0, 0);
    } else {
        usb_send_empty(0);
    }
}

// ==== Job State Processing ====

static void process_job_state(void)
{
    uint8_t i;

    switch(jobState) {
        case 0:  // Idle
            break;

        case 1:  // Debug SPI (bitbang, slow)
            sendBuffer[0] = spi_debug_transfer(rxBuffer[0]);
            sendBuffer[8] = 1;
            jobState = 0;
            break;

        case 2:  // OneWire reset pulse
            sendBuffer[0] = onewire_reset();
            sendBuffer[8] = 1;
            jobState = 0;
            break;

        case 3:  // OneWire send byte
            onewire_write_byte(rxBuffer[0]);
            jobState = 0;
            break;

        case 4:  // OneWire read byte
            sendBuffer[0] = onewire_read_byte();
            sendBuffer[8] = 1;
            jobState = 0;
            break;

        case 5:  // OneWire read bit
            sendBuffer[0] = onewire_read_bit();
            sendBuffer[8] = 1;
            jobState = 0;
            break;

        case 6:  // OneWire write bit
            onewire_write_bit(rxBuffer[0]);
            jobState = 0;
            break;

        case 7:  // Multiple SPI send/receive
            if (rxBuffer[0]) {
                // Auto chip select - drive CS low
                set_pin_low(5);  // PIN3 as CS
            }
            for (i = 0; i < rxBuffer[1]; i++) {
                sendBuffer[i] = spi_transfer(rxBuffer[2 + i]);
            }
            if (rxBuffer[0]) {
                set_pin_high(5);  // CS high
            }
            sendBuffer[8] = rxBuffer[1];
            jobState = 0;
            break;

        case 8:  // I2C init
            i2c_init();
            jobState = 0;
            break;

        case 9:  // I2C send address
            i2c_init();
            sendBuffer[0] = i2c_start(rxBuffer[0]);
            sendBuffer[8] = 1;
            jobState = 0;
            break;

        case 10:  // I2C send bytes
            for (i = 0; i < rxBuffer[0]; i++) {
                sendBuffer[0] = i2c_write(rxBuffer[2 + i]);
            }
            if (rxBuffer[1]) {
                i2c_stop();
            }
            sendBuffer[8] = 1;
            jobState = 0;
            break;

        case 11:  // I2C read bytes
            if (rxBuffer[0]) {  // End with NACK?
                for (i = 0; i < rxBuffer[1] - 1; i++) {
                    sendBuffer[i] = i2c_read(1);  // ACK
                }
                sendBuffer[i] = i2c_read(0);  // NACK
            } else {
                for (i = 0; i < rxBuffer[1]; i++) {
                    sendBuffer[i] = i2c_read(1);  // Always ACK
                }
            }
            if (rxBuffer[2]) {
                i2c_stop();
            }
            sendBuffer[8] = rxBuffer[1];
            jobState = 0;
            break;

        case 17:  // WS2812 write
            Delay_Ms(1);  // Ensure USB is done
            ws2812_flush(rxBuffer[0]);
            jobState = 0;
            break;

        default:
            jobState = 0;
            break;
    }
}

// ==== Soft PWM Processing ====

static void process_soft_pwm(void)
{
    if (!softPWM) return;

    if (counter == 0) {
        cmp0 = compare0;
        cmp1 = compare1;
        cmp2 = compare2;
    } else {
        // PIN4 (PC4)
        if (counter > cmp0) {
            GPIOC->BCR = (1 << 4);
        } else {
            GPIOC->BSHR = (1 << 4);
        }
        // PIN1 (PC1)
        if (counter > cmp1) {
            GPIOC->BCR = (1 << 1);
        } else {
            GPIOC->BSHR = (1 << 1);
        }
        // PIN2 (PC2)
        if (counter > cmp2) {
            GPIOC->BCR = (1 << 2);
        } else {
            GPIOC->BSHR = (1 << 2);
        }
    }
    counter++;
}

// ==== Main Entry Point ====

int main(void)
{
    // System initialization
    SystemInit();

    // Enable GPIO clocks
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA |
                      RCC_APB2Periph_GPIOC |
                      RCC_APB2Periph_GPIOD;

    // Small delay for USB re-enumeration
    Delay_Ms(2);

    // Initialize USB
    usb_setup();

    // Initialize all pins as floating inputs
    power_down_pins();

    // Initialize default values
    i2c_set_delay(5);
    spi_set_delay(0);

    // Main loop
    while(1) {
        // Process job states
        process_job_state();

        // Process soft PWM
        process_soft_pwm();
    }

    return 0;
}
