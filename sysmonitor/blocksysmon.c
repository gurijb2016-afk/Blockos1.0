#include "blocksysmon.h"
#include "../../kernel/generic.h"
#include "../../kernel/process.h"
#include "../../gui/gui.h"
#include "../../wm/blockwm.h"

#define HIST_SIZE 60

static uint8_t  cpu_hist[HIST_SIZE];
static uint8_t  mem_hist[HIST_SIZE];
static uint32_t hist_idx = 0;
static blockwm_window_t* g_win = NULL;

static struct {
    uint8_t  cpu;
    uint32_t mem_total, mem_used, mem_free;
    uint32_t proc_count;
    uint64_t uptime;
} g_info;

bool sysmon_init(void) {
    memset(cpu_hist, 0, sizeof(cpu_hist));
    memset(mem_hist, 0, sizeof(mem_hist));
    g_win = blockwm_create_window(
        "BlockSysMon - Rendszer Monitor", 50, 50, 700, 500,
        BLOCKWM_FLAG_TITLEBAR|BLOCKWM_FLAG_RESIZABLE|BLOCKWM_FLAG_SHADOW);
    return g_win != NULL;
}

void sysmon_update(void) {
    extern uint32_t mem_get_total(void);
    extern uint32_t mem_get_free(void);
    g_info.mem_total  = mem_get_total();
    g_info.mem_free   = mem_get_free();
    g_info.mem_used   = g_info.mem_total - g_info.mem_free;
    g_info.uptime     = kernel_get_ticks() / 100;
    g_info.proc_count = 0;
    extern process_t process_table[]; extern uint32_t process_count;
    for (uint32_t i = 0; i < process_count; i++)
        if (process_table[i].state != PROCESS_STATE_DEAD) g_info.proc_count++;
    g_info.cpu = (uint8_t)(g_info.proc_count * 5);
    if (g_info.cpu > 100) g_info.cpu = 100;
    cpu_hist[hist_idx % HIST_SIZE] = g_info.cpu;
    mem_hist[hist_idx % HIST_SIZE] =
        (uint8_t)(g_info.mem_used * 100 / (g_info.mem_total ? g_info.mem_total : 1));
    hist_idx++;
}

static void draw_graph(int x, int y, int w, int h, uint8_t* data,
                        uint32_t idx, uint32_t color, const char* label) {
    gui_draw_rect(x, y, w, h, 0x0d1117);
    gui_draw_rect_outline(x, y, w, h, 1, 0x21262d);
    for (int i = 1; i < 4; i++)
        gui_draw_rect(x, y + h*i/4, w, 1, 0x1f2937);
    for (uint32_t i = 1; i < HIST_SIZE; i++) {
        int x1 = x + (i-1)*w/HIST_SIZE;
        int x2 = x + i*w/HIST_SIZE;
        int y1 = y + h - data[(idx-HIST_SIZE+i-1)%HIST_SIZE]*h/100;
        int y2 = y + h - data[(idx-HIST_SIZE+i)%HIST_SIZE]*h/100;
        gui_draw_rect(x1, y1<y2?y1:y2, x2-x1+1, abs(y2-y1)+1, color);
    }
    char val[32];
    snprintf(val, 32, "%s: %u%%", label, data[(idx-1)%HIST_SIZE]);
    gui_draw_string(x+4, y+4, val, color);
}

void sysmon_draw(void) {
    gui_draw_rect(0, 0, 700, 500, 0x0d1117);
    gui_draw_rect(0, 0, 700, 38, 0x161b22);
    gui_draw_string(10, 12, "BlockOS Rendszer Monitor", 0xe0e0e0);
    char up[64];
    snprintf(up, 64, "Uptime: %llu mp | Proc: %u", g_info.uptime, g_info.proc_count);
    gui_draw_string(320, 12, up, 0x8b949e);

    draw_graph(10,  48, 330, 110, cpu_hist, hist_idx, 0x22c55e, "CPU");
    draw_graph(355, 48, 330, 110, mem_hist, hist_idx, 0x3b82f6, "RAM");

    char mem[128];
    snprintf(mem, 128, "RAM: %u MB / %u MB | Szabad: %u MB",
        g_info.mem_used/1024/1024, g_info.mem_total/1024/1024, g_info.mem_free/1024/1024);
    gui_draw_string(10, 168, mem, 0x8b949e);

    gui_draw_rect(0, 185, 700, 22, 0x161b22);
    gui_draw_string(10, 190, "PID",     0x8b949e);
    gui_draw_string(55, 190, "Név",     0x8b949e);
    gui_draw_string(240,190, "Állapot", 0x8b949e);
    gui_draw_string(350,190, "RAM",     0x8b949e);
    gui_draw_string(430,190, "Prio",    0x8b949e);

    extern process_t process_table[]; extern uint32_t process_count;
    int row = 0;
    for (uint32_t i = 0; i < process_count && row < 14; i++) {
        process_t* p = &process_table[i];
        if (p->state == PROCESS_STATE_DEAD) continue;
        int ry = 207 + row*20;
        if (row%2==0) gui_draw_rect(0, ry, 700, 20, 0x111827);
        const char* st = "?"; uint32_t sc = 0x8b949e;
        switch(p->state) {
            case PROCESS_STATE_READY:   st="Fut";    sc=0x22c55e; break;
            case PROCESS_STATE_BLOCKED: st="Vár";    sc=0xf59e0b; break;
            case PROCESS_STATE_ZOMBIE:  st="Zombie"; sc=0xef4444; break;
            case PROCESS_STATE_NEW:     st="Új";     sc=0x3b82f6; break;
        }
        char pid[8], ram[16], prio[8];
        snprintf(pid,  8,  "%u", p->pid);
        snprintf(ram,  16, "%u KB", (p->heap_end-p->code_start)/1024);
        snprintf(prio, 8,  "%d", p->priority);
        gui_draw_string(10,  ry+3, pid,    0xe0e0e0);
        gui_draw_string(55,  ry+3, p->name,0xe0e0e0);
        gui_draw_string(240, ry+3, st,     sc);
        gui_draw_string(350, ry+3, ram,    0x8b949e);
        gui_draw_string(430, ry+3, prio,   0x8b949e);
        row++;
    }

    gui_draw_rect(0, 480, 700, 20, 0x161b22);
    char st[128];
    snprintf(st, 128, "CPU: %u%% | RAM: %u/%u MB | Procs: %u",
        g_info.cpu, g_info.mem_used/1024/1024, g_info.mem_total/1024/1024, g_info.proc_count);
    gui_draw_string(10, 484, st, 0x8b949e);
}
