#include "timer.h"
#include "../kernel/generic.h"
static uint64_t g_ticks = 0;
static timer_callback_t g_cbs[16];
static uint32_t g_cb_count = 0;
void timer_init(uint32_t hz) {
    uint32_t div = PIT_FREQ / hz;
    outb(0x43, 0x36);
    outb(0x40, div & 0xFF);
    outb(0x40, (div>>8) & 0xFF);
}
void timer_irq_handler(void) {
    g_ticks++; kernel_tick();
    for (uint32_t i=0;i<g_cb_count;i++) if(g_cbs[i]) g_cbs[i]();
    outb(0x20, 0x20);
}
uint64_t timer_get_ticks(void){return g_ticks;}
uint64_t timer_get_seconds(void){return g_ticks/TIMER_HZ;}
void timer_sleep(uint32_t ms){
    uint64_t end=g_ticks+(ms*TIMER_HZ/1000);
    while(g_ticks<end) __asm__ volatile("hlt");
}
void timer_register_callback(timer_callback_t cb){
    if(g_cb_count<16) g_cbs[g_cb_count++]=cb;
}
