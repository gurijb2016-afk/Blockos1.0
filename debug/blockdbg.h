#ifndef BLOCKDBG_H
#define BLOCKDBG_H
#include "../../kernel/types.h"
#define DBG_LEVEL_INFO  0
#define DBG_LEVEL_WARN  1
#define DBG_LEVEL_ERROR 2
#define DBG_LEVEL_PANIC 3
#define DBG_LEVEL_TRACE 4
typedef struct { uint32_t address; bool active; uint8_t original_byte; uint32_t hit_count; } breakpoint_t;
typedef struct { uint32_t eip, esp, ebp; char function[64]; } stack_frame_t;
typedef struct { uint8_t level; char message[256]; uint32_t timestamp; uint32_t pid; } dbg_log_entry_t;
void     dbg_init(void);
void     dbg_log(uint8_t level, const char* msg);
void     dbg_info(const char* msg);
void     dbg_warn(const char* msg);
void     dbg_error(const char* msg);
void     dbg_trace(const char* msg);
int      dbg_add_breakpoint(uint32_t address);
bool     dbg_remove_breakpoint(int idx);
uint32_t dbg_stack_trace(stack_frame_t* frames, uint32_t max);
void     dbg_dump_registers(void);
void     dbg_assert(bool cond, const char* msg);
uint32_t dbg_get_log(dbg_log_entry_t* out, uint32_t max);
#endif
