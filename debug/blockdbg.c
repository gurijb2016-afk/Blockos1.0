#include "blockdbg.h"
#include "../../kernel/generic.h"
#include "../../kernel/process.h"

#define DBG_MAX_BP  32
#define DBG_LOG_SZ  4096

static struct {
    bool         active;
    breakpoint_t bp[DBG_MAX_BP];
    uint32_t     bp_count;
    dbg_log_entry_t log[DBG_LOG_SZ];
    uint32_t     log_head;
    uint32_t     log_count;
} g_dbg;

void dbg_init(void) { memset(&g_dbg, 0, sizeof(g_dbg)); g_dbg.active = true; }

void dbg_log(uint8_t level, const char* msg) {
    if (!g_dbg.active) return;
    dbg_log_entry_t* e = &g_dbg.log[g_dbg.log_head % DBG_LOG_SZ];
    e->level = level; e->timestamp = (uint32_t)kernel_get_ticks();
    strncpy(e->message, msg, 255);
    process_t* c = process_get_current(); e->pid = c ? c->pid : 0;
    g_dbg.log_head++;
    if (g_dbg.log_count < DBG_LOG_SZ) g_dbg.log_count++;
}

void dbg_info(const char* m)  { dbg_log(DBG_LEVEL_INFO,  m); }
void dbg_warn(const char* m)  { dbg_log(DBG_LEVEL_WARN,  m); }
void dbg_error(const char* m) { dbg_log(DBG_LEVEL_ERROR, m); }
void dbg_trace(const char* m) { dbg_log(DBG_LEVEL_TRACE, m); }

int dbg_add_breakpoint(uint32_t addr) {
    if (g_dbg.bp_count >= DBG_MAX_BP) return -1;
    breakpoint_t* bp = &g_dbg.bp[g_dbg.bp_count++];
    bp->address = addr; bp->active = true;
    bp->original_byte = *(uint8_t*)addr;
    *(uint8_t*)addr = 0xCC;
    return g_dbg.bp_count - 1;
}

bool dbg_remove_breakpoint(int idx) {
    if (idx < 0 || idx >= (int)g_dbg.bp_count) return false;
    *(uint8_t*)g_dbg.bp[idx].address = g_dbg.bp[idx].original_byte;
    g_dbg.bp[idx].active = false;
    return true;
}

uint32_t dbg_stack_trace(stack_frame_t* frames, uint32_t max) {
    uint32_t* ebp; __asm__ volatile ("mov %%ebp,%0":"=r"(ebp));
    uint32_t n = 0;
    while (ebp && n < max) {
        frames[n].ebp = (uint32_t)ebp;
        frames[n].eip = *(ebp+1);
        snprintf(frames[n].function, 64, "0x%x", frames[n].eip);
        ebp = (uint32_t*)*ebp; n++;
    }
    return n;
}

void dbg_dump_registers(void) {
    uint32_t a,b,c,d,sp,bp,si,di;
    __asm__ volatile("mov %%eax,%0\nmov %%ebx,%1\nmov %%ecx,%2\nmov %%edx,%3\nmov %%esp,%4\nmov %%ebp,%5\nmov %%esi,%6\nmov %%edi,%7\n"
        :"=r"(a),"=r"(b),"=r"(c),"=r"(d),"=r"(sp),"=r"(bp),"=r"(si),"=r"(di));
    char buf[256];
    snprintf(buf,256,"EAX=%08x EBX=%08x ECX=%08x EDX=%08x ESP=%08x EBP=%08x",a,b,c,d,sp,bp);
    dbg_log(DBG_LEVEL_TRACE, buf);
}

void dbg_assert(bool cond, const char* msg) { if (!cond) { dbg_error(msg); kernel_panic(msg); } }

uint32_t dbg_get_log(dbg_log_entry_t* out, uint32_t max) {
    uint32_t n = g_dbg.log_count < max ? g_dbg.log_count : max;
    for (uint32_t i = 0; i < n; i++)
        out[i] = g_dbg.log[(g_dbg.log_head - n + i) % DBG_LOG_SZ];
    return n;
}
