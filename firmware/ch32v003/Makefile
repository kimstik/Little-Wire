# Little-Wire CH32V003 Firmware Makefile
# @author kimstik
# assisted by intelligence
#
# Usage:
#   make          - Build firmware
#   make flash    - Build and flash to device
#   make clean    - Clean build artifacts
#
# Prerequisites:
#   - RISC-V GCC toolchain
#   - minichlink for flashing

all : flash

TARGET := littlewire_ch32v003
CH32FUN := rv003usb/ch32v003fun/ch32fun
TARGET_MCU := CH32V003

# Source files
ADDITIONAL_C_FILES += rv003usb/rv003usb/rv003usb.S
ADDITIONAL_C_FILES += rv003usb/rv003usb/rv003usb.c
ADDITIONAL_C_FILES += peripherals/adc.c
ADDITIONAL_C_FILES += peripherals/pwm.c
ADDITIONAL_C_FILES += peripherals/spi.c
ADDITIONAL_C_FILES += peripherals/i2c.c
ADDITIONAL_C_FILES += peripherals/onewire.c
ADDITIONAL_C_FILES += peripherals/ws2812.c

# Include paths
EXTRA_CFLAGS := -I.
EXTRA_CFLAGS += -Irv003usb/lib
EXTRA_CFLAGS += -Irv003usb/rv003usb
EXTRA_CFLAGS += -Iperipherals

# Optimization for size
EXTRA_CFLAGS += -Os

# Include ch32v003fun build system
include $(CH32FUN)/ch32fun.mk

flash : cv_flash
clean : cv_clean

# Additional targets

# Print binary size
size : $(TARGET).elf
	$(PREFIX)size $<

# Disassemble
disasm : $(TARGET).elf
	$(PREFIX)objdump -d $< > $(TARGET).disasm

# Generate hex file (for other programmers)
hex : $(TARGET).elf
	$(PREFIX)objcopy -O ihex $< $(TARGET).hex

.PHONY: all flash clean size disasm hex
