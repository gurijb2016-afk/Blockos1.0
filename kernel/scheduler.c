#include "scheduler.h"
#include "generic.h"

static int       g_mode = SCHED_ROUND_ROBIN;
static process_t* g_queue[256];
static uint32_t  g_queue_head = 0;
static uint32_t  g_queue_tail = 0;
static uint32_t  g_queue_count = 0;
static uint32_t  g_tick = 0;

void scheduler_init(int mode) {
    g_mode = mode;
    g_queue_head = g_queue_tail = g_queue_count = 0;
    memset(g_queue, 0, sizeof(g_queue));
}

void scheduler_add(process_t* proc) {
    if (!proc || g_queue_count >= 256) return;
    g_queue[g_queue_tail % 256] = proc;
    g_queue_tail++;
    g_queue_count++;
}

void scheduler_remove(process_t* proc) {
    for (uint32_t i = 0; i < g_queue_count; i++) {
        if (g_queue[(g_queue_head + i) % 256] == proc) {
            g_queue[(g_queue_head + i) % 256] = NULL;
            g_queue_count--;
            return;
        }
    }
}

process_t* scheduler_next(void) {
    if (g_queue_count == 0) return NULL;

    if (g_mode == SCHED_ROUND_ROBIN) {
        /* Round Robin */
        for (uint32_t i = 0; i < 256; i++) {
            process_t* p = g_queue[(g_queue_head + i) % 256];
            if (p && p->state == PROCESS_STATE_READY) {
                g_queue_head = (g_queue_head + i + 1) % 256;
                return p;
            }
        }
    } else if (g_mode == SCHED_PRIORITY) {
        /* Priority alapú */
        process_t* best = NULL;
        int best_prio = -1000;
        for (uint32_t i = 0; i < 256; i++) {
            process_t* p = g_queue[(g_queue_head + i) % 256];
            if (p && p->state == PROCESS_STATE_READY && p->priority > best_prio) {
                best = p; best_prio = p->priority;
            }
        }
        return best;
    }
    return NULL;
}

void scheduler_tick(void) {
    g_tick++;
    process_t* cur = process_get_current();
    if (!cur) { process_set_current(scheduler_next()); return; }

    cur->jiffies_used++;
    cur->time_slice--;

    if (cur->time_slice <= 0) {
        cur->time_slice = SCHED_TIME_SLICE;
        process_t* next = scheduler_next();
        if (next && next != cur) {
            cur->state = PROCESS_STATE_READY;
            process_set_current(next);
            next->state = PROCESS_STATE_READY;
        }
    }
}

void scheduler_yield(void) {
    process_t* cur = process_get_current();
    if (cur) { cur->time_slice = 0; scheduler_tick(); }
}

void scheduler_block(process_t* proc) {
    if (proc) proc->state = PROCESS_STATE_BLOCKED;
}

void scheduler_unblock(process_t* proc) {
    if (proc) proc->state = PROCESS_STATE_READY;
}

void scheduler_set_mode(int mode) { g_mode = mode; }

uint32_t scheduler_get_running_count(void) { return g_queue_count; }
