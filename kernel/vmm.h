#ifndef VMM_H
#define VMM_H
#include "types.h"
#define PAGE_SIZE     4096
#define PAGE_PRESENT  0x01
#define PAGE_WRITABLE 0x02
#define PAGE_USER     0x04
#define PAGE_DIRTY    0x40
#define PAGE_ACCESSED 0x20
#define KERNEL_BASE   0xC0000000
typedef uint32_t page_t;
typedef struct { page_t entries[1024]; } page_table_t;
typedef struct { page_t entries[1024]; } page_dir_t;
void  vmm_init(void);
void  vmm_map(uint32_t virt, uint32_t phys, uint32_t flags);
void  vmm_unmap(uint32_t virt);
uint32_t vmm_get_phys(uint32_t virt);
void  vmm_switch(page_dir_t* dir);
page_dir_t* vmm_create_dir(void);
void  vmm_destroy_dir(page_dir_t* dir);
bool  vmm_is_mapped(uint32_t virt);
#endif
