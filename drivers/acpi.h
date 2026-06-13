#ifndef ACPI_H
#define ACPI_H
#include "../kernel/types.h"
typedef struct { char sig[8]; uint8_t checksum; char oem[6]; uint8_t rev; uint32_t rsdt; } __attribute__((packed)) rsdp_t;
bool acpi_init(void);
void acpi_shutdown(void);
void acpi_reboot(void);
bool acpi_is_available(void);
#endif
