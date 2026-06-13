#include "vmm.h"
#include "generic.h"
static page_dir_t* g_kernel_dir = NULL;
static page_dir_t* g_current_dir = NULL;

void vmm_init(void) {
    g_kernel_dir = (page_dir_t*)malloc(sizeof(page_dir_t));
    if (!g_kernel_dir) return;
    memset(g_kernel_dir, 0, sizeof(page_dir_t));

    /* Identity map első 4MB */
    for (uint32_t i = 0; i < 1024; i++)
        vmm_map(i * PAGE_SIZE, i * PAGE_SIZE, PAGE_PRESENT | PAGE_WRITABLE);

    vmm_switch(g_kernel_dir);
    /* Paging engedélyezés */
    uint32_t cr0;
    __asm__ volatile("mov %%cr0,%0":"=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0,%%cr0"::"r"(cr0));
}

void vmm_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    if (!g_kernel_dir) return;
    uint32_t dir_idx  = virt >> 22;
    uint32_t tbl_idx  = (virt >> 12) & 0x3FF;
    page_table_t* tbl;

    if (!(g_kernel_dir->entries[dir_idx] & PAGE_PRESENT)) {
        tbl = (page_table_t*)malloc(sizeof(page_table_t));
        if (!tbl) return;
        memset(tbl, 0, sizeof(page_table_t));
        g_kernel_dir->entries[dir_idx] = ((uint32_t)tbl) | PAGE_PRESENT | PAGE_WRITABLE;
    } else {
        tbl = (page_table_t*)(g_kernel_dir->entries[dir_idx] & ~0xFFF);
    }
    tbl->entries[tbl_idx] = (phys & ~0xFFF) | (flags & 0xFFF);
}

void vmm_unmap(uint32_t virt) {
    uint32_t dir_idx = virt >> 22;
    uint32_t tbl_idx = (virt >> 12) & 0x3FF;
    if (!(g_kernel_dir->entries[dir_idx] & PAGE_PRESENT)) return;
    page_table_t* tbl = (page_table_t*)(g_kernel_dir->entries[dir_idx] & ~0xFFF);
    tbl->entries[tbl_idx] = 0;
    __asm__ volatile("invlpg (%0)"::"r"(virt):"memory");
}

uint32_t vmm_get_phys(uint32_t virt) {
    uint32_t dir_idx = virt >> 22;
    uint32_t tbl_idx = (virt >> 12) & 0x3FF;
    if (!g_kernel_dir || !(g_kernel_dir->entries[dir_idx] & PAGE_PRESENT)) return 0;
    page_table_t* tbl = (page_table_t*)(g_kernel_dir->entries[dir_idx] & ~0xFFF);
    return (tbl->entries[tbl_idx] & ~0xFFF) | (virt & 0xFFF);
}

void vmm_switch(page_dir_t* dir) {
    g_current_dir = dir;
    __asm__ volatile("mov %0,%%cr3"::"r"((uint32_t)dir));
}

page_dir_t* vmm_create_dir(void) {
    page_dir_t* dir = (page_dir_t*)malloc(sizeof(page_dir_t));
    if (!dir) return NULL;
    memcpy(dir, g_kernel_dir, sizeof(page_dir_t));
    return dir;
}

void vmm_destroy_dir(page_dir_t* dir) { if(dir) free(dir); }
bool vmm_is_mapped(uint32_t virt) { return vmm_get_phys(virt) != 0; }
