#include "acpi.h"
#include "../kernel/generic.h"
static bool g_avail=false;
static rsdp_t* find_rsdp(void){
    for(uint32_t a=0xE0000;a<0x100000;a+=16){
        rsdp_t* r=(rsdp_t*)a;
        if(memcmp(r->sig,"RSD PTR ",8)==0) return r;
    }
    return NULL;
}
bool acpi_init(void){g_avail=find_rsdp()!=NULL;return g_avail;}
bool acpi_is_available(void){return g_avail;}
void acpi_shutdown(void){
    outw(0x604,0x2000); /* QEMU */
    outw(0xB004,0x2000); /* Bochs */
    outw(0x4004,0x3400); /* VirtualBox */
    __asm__ volatile("cli;hlt");
    while(1);
}
void acpi_reboot(void){
    uint8_t g=0x02;
    while(g&0x02) g=inb(0x64);
    outb(0x64,0xFE);
    __asm__ volatile("cli");
    while(1);
}
