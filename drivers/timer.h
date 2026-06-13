#ifndef TIMER_H
#define TIMER_H
#include "../kernel/types.h"
#define PIT_FREQ 1193182
#define TIMER_HZ 100
typedef void (*timer_callback_t)(void);
void     timer_init(uint32_t hz);
void     timer_irq_handler(void);
uint64_t timer_get_ticks(void);
uint64_t timer_get_seconds(void);
void     timer_sleep(uint32_t ms);
void     timer_register_callback(timer_callback_t cb);
#endif
