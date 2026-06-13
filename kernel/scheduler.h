#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "types.h"
#include "process.h"
#define SCHED_ROUND_ROBIN  0
#define SCHED_PRIORITY     1
#define SCHED_MLFQ        2
#define SCHED_TIME_SLICE   20
void scheduler_init(int mode);
void scheduler_tick(void);
void scheduler_add(process_t* proc);
void scheduler_remove(process_t* proc);
process_t* scheduler_next(void);
void scheduler_yield(void);
void scheduler_block(process_t* proc);
void scheduler_unblock(process_t* proc);
void scheduler_set_mode(int mode);
uint32_t scheduler_get_running_count(void);
#endif
