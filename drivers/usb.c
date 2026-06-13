#include "usb.h"
#include "../kernel/generic.h"
static usb_controller_t g_usb;
bool usb_init(void){memset(&g_usb,0,sizeof(g_usb));g_usb.initialized=true;return true;}
bool usb_scan_devices(void){return g_usb.initialized;}
usb_device_t* usb_get_device(uint8_t addr){
    for(uint32_t i=0;i<g_usb.device_count;i++)
        if(g_usb.devices[i].address==addr&&g_usb.devices[i].connected)
            return &g_usb.devices[i];
    return NULL;
}
bool usb_send(usb_device_t* d,uint8_t* data,uint32_t size){return d&&d->connected;}
bool usb_recv(usb_device_t* d,uint8_t* buf,uint32_t* size){if(!d||!d->connected)return false;*size=0;return true;}
uint32_t usb_get_device_count(void){return g_usb.device_count;}
