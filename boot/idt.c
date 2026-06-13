#include "../kernel/types.h"
#include "../kernel/generic.h"

/* IDT - Interrupt Descriptor Table */

#define IDT_ENTRIES 256

typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags | 0x60;
}

extern void idt_flush(uint32_t);
extern void isr0(void);
extern void isr1(void);
extern void irq0(void);
extern void irq1(void);
extern void irq12(void);

void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    memset(&idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

    /* PIC újraprogramozás */
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0x00); outb(0xA1, 0x00);

    /* IRQ-k */
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E); // Timer
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E); // Keyboard
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E); // PS/2 Mouse

    idt_flush((uint32_t)&idt_ptr);
}

/* IRQ kezelők */
void irq0_handler(void) {
    kernel_tick();
    outb(0x20, 0x20);
}

void irq1_handler(void) {
    uint8_t key = inb(0x60);
    (void)key;
    outb(0x20, 0x20);
}

void irq12_handler(void) {
    /* PS/2 egér interrupt */
    extern void ps2_mouse_irq_handler(void);
    ps2_mouse_irq_handler();
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
