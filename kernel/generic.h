#ifndef GENERIC_H
#define GENERIC_H

#include "types.h"

/* I/O */
void     outb(uint16_t port, uint8_t value);
uint8_t  inb(uint16_t port);
void     outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);
void     outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);
void     io_wait(void);

/* Kernel */
void     kernel_tick(void);
uint64_t kernel_get_ticks(void);
void     kernel_panic(const char* msg);
void     kernel_loop(void);

/* Memory */
uint32_t mem_get_total(void);
uint32_t mem_get_free(void);

#endif
