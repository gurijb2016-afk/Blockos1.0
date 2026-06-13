#ifndef USB_H
#define USB_H
#include "../kernel/types.h"
#define USB_MAX_DEVICES 16
#define USB_CLASS_HID   0x03
#define USB_CLASS_MASS  0x08
#define USB_CLASS_HUB   0x09
typedef struct {
    uint8_t address, class_code;
    uint16_t vendor_id, product_id, max_packet;
    bool connected; char name[64];
} usb_device_t;
typedef struct {
    usb_device_t devices[USB_MAX_DEVICES];
    uint32_t device_count; bool initialized;
} usb_controller_t;
bool usb_init(void);
bool usb_scan_devices(void);
usb_device_t* usb_get_device(uint8_t address);
bool usb_send(usb_device_t* dev, uint8_t* data, uint32_t size);
bool usb_recv(usb_device_t* dev, uint8_t* buf, uint32_t* size);
uint32_t usb_get_device_count(void);
#endif
