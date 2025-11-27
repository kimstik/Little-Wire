/**
 * @file usb_config.h
 * @brief USB configuration for Little-Wire CH32V003
 * @author kimstik
 * assisted by intelligence
 *
 * USB protocol compatible with original Little-Wire (ATtiny85)
 */

#ifndef _USB_CONFIG_H
#define _USB_CONFIG_H

// Endpoints: EP0 (control) only - Little-Wire uses control transfers
#define ENDPOINTS 1

// USB Pin Configuration for CH32V003
// Using Port D for USB signals
#define USB_PORT   D
#define USB_PIN_DP 3   // GPIO D3 - USB D+
#define USB_PIN_DM 4   // GPIO D4 - USB D-
#define USB_PIN_DPU 5  // GPIO D5 - 1.5k Pull-Up control

// rv003usb feature flags
#define RV003USB_OPTIMIZE_FLASH    0  // Don't optimize, prioritize compatibility
#define RV003USB_EVENT_DEBUGGING   0
#define RV003USB_DEBUG_TIMING      0
#define RV003USB_HANDLE_IN_REQUEST 0
#define RV003USB_OTHER_CONTROL     1  // REQUIRED: Handle Little-Wire protocol
#define RV003USB_HANDLE_USER_DATA  0
#define RV003USB_HID_FEATURES      0
#define RV003USB_USE_REBOOT_FEATURE_REPORT 0

#ifndef __ASSEMBLER__

#ifdef INSTANCE_DESCRIPTORS

// USB Device Descriptor - Vendor-specific class (like original Little-Wire)
static const uint8_t device_descriptor[] = {
    18,         // bLength
    1,          // bDescriptorType (Device)
    0x10, 0x01, // bcdUSB 1.1
    0xFF,       // bDeviceClass (Vendor-specific)
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    0x08,       // bMaxPacketSize0 (8 bytes for Low-Speed)
    0x81, 0x17, // idVendor  = 0x1781 (same as original Little-Wire)
    0x9f, 0x0c, // idProduct = 0x0c9f (same as original Little-Wire)
    0x20, 0x00, // bcdDevice = 2.0 (CH32V003 version)
    1,          // iManufacturer
    2,          // iProduct
    3,          // iSerialNumber
    1           // bNumConfigurations
};

// USB Configuration Descriptor
static const uint8_t config_descriptor[] = {
    // Configuration Descriptor
    9,          // bLength
    2,          // bDescriptorType (Configuration)
    18, 0x00,   // wTotalLength (9 + 9 = 18)
    0x01,       // bNumInterfaces
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration
    0x80,       // bmAttributes (bus-powered)
    100,        // bMaxPower (200mA)

    // Interface Descriptor
    9,          // bLength
    4,          // bDescriptorType (Interface)
    0,          // bInterfaceNumber
    0,          // bAlternateSetting
    0,          // bNumEndpoints (control only)
    0xFF,       // bInterfaceClass (Vendor-specific)
    0x00,       // bInterfaceSubClass
    0x00,       // bInterfaceProtocol
    0           // iInterface
};

// String Descriptors
#define STR_MANUFACTURER u"Little-Wire"
#define STR_PRODUCT      u"Little-Wire CH32V003"
#define STR_SERIAL       u"001"

struct usb_string_descriptor_struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wString[];
};

const static struct usb_string_descriptor_struct string0 __attribute__((section(".rodata"))) = {
    4,
    3,
    {0x0409}
};

const static struct usb_string_descriptor_struct string1 __attribute__((section(".rodata"))) = {
    sizeof(STR_MANUFACTURER),
    3,
    STR_MANUFACTURER
};

const static struct usb_string_descriptor_struct string2 __attribute__((section(".rodata"))) = {
    sizeof(STR_PRODUCT),
    3,
    STR_PRODUCT
};

const static struct usb_string_descriptor_struct string3 __attribute__((section(".rodata"))) = {
    sizeof(STR_SERIAL),
    3,
    STR_SERIAL
};

// Descriptor list for rv003usb
const static struct descriptor_list_struct {
    uint32_t    lIndexValue;
    const uint8_t   *addr;
    uint8_t     length;
} descriptor_list[] = {
    {0x00000100, device_descriptor, sizeof(device_descriptor)},
    {0x00000200, config_descriptor, sizeof(config_descriptor)},
    {0x00000300, (const uint8_t *)&string0, 4},
    {0x04090301, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
    {0x04090302, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},
    {0x04090303, (const uint8_t *)&string3, sizeof(STR_SERIAL)}
};

#define DESCRIPTOR_LIST_ENTRIES ((sizeof(descriptor_list))/(sizeof(struct descriptor_list_struct)))

#endif // INSTANCE_DESCRIPTORS

#endif // __ASSEMBLER__

#endif // _USB_CONFIG_H
