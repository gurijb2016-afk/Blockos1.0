#include "taskbar.h"
#include "../gui/gui.h"
#include "../kernel/generic.h"

void taskbar_init(taskbar_t* tb, uint32_t sw, uint32_t sh, uint8_t pos) {
    memset(tb, 0, sizeof(taskbar_t));
    tb->w = sw;
    tb->h = TASKBAR_HEIGHT;
    tb->position = pos;
    tb->x = 0;
    tb->y = (pos == TASKBAR_POSITION_BOTTOM) ? sh - TASKBAR_HEIGHT : 0;
    tb->clock_visible  = true;
    tb->systray_visible = true;
    tb->hovered_btn = -1;
}

void taskbar_add_window(taskbar_t* tb, blockwm_window_t* win) {
    if (!tb || !win || tb->btn_count >= TASKBAR_MAX_BTNS) return;
    taskbar_btn_t* btn = &tb->buttons[tb->btn_count];
    strncpy(btn->label, win->title, 31);
    btn->window_id = win->id;
    btn->active    = true;
    btn->hovered   = false;
    /* Gomb szélesség számítás */
    int max_w = (tb->w - 200) / (tb->btn_count + 1);
    if (max_w > 160) max_w = 160;
    if (max_w < 60)  max_w = 60;
    int start_x = 8;
    for (uint32_t i = 0; i <= tb->btn_count; i++) {
        tb->buttons[i].x = start_x + i * (max_w + 4);
        tb->buttons[i].w = max_w;
    }
    tb->btn_count++;
}

void taskbar_remove_window(taskbar_t* tb, int window_id) {
    if (!tb) return;
    for (uint32_t i = 0; i < tb->btn_count; i++) {
        if (tb->buttons[i].window_id == window_id) {
            for (uint32_t j = i; j < tb->btn_count - 1; j++)
                tb->buttons[j] = tb->buttons[j+1];
            tb->btn_count--;
            return;
        }
    }
}

void taskbar_draw(taskbar_t* tb) {
    if (!tb) return;

    /* Háttér */
    gui_draw_rect(tb->x, tb->y, tb->w, tb->h, 0x111827);
    /* Felső vonal */
    gui_draw_rect(tb->x, tb->y, tb->w, 1, 0x21262d);

    /* Start gomb */
    gui_draw_rect(tb->x + 4, tb->y + 5, 70, 30, 0x2563eb);
    gui_draw_string(tb->x + 14, tb->y + 12, "BlockOS", 0xffffff);

    /* Ablak gombok */
    for (uint32_t i = 0; i < tb->btn_count; i++) {
        taskbar_btn_t* btn = &tb->buttons[i];
        bool hov = (tb->hovered_btn == (int)i);
        uint32_t bg = hov ? 0x21262d : 0x161b22;
        gui_draw_rect(btn->x, tb->y + 5, btn->w, 30, bg);
        gui_draw_rect_outline(btn->x, tb->y + 5, btn->w, 30, 1, 0x30363d);
        /* Label rövidítés */
        char short_label[20];
        strncpy(short_label, btn->label, 16);
        if (strlen(btn->label) > 16) { short_label[14]='.'; short_label[15]='.'; short_label[16]=0; }
        gui_draw_string(btn->x + 6, tb->y + 13, short_label, 0xe0e0e0);
    }

    /* Óra */
    uint64_t ticks = kernel_get_ticks();
    uint64_t secs  = ticks / 100;
    uint64_t mins  = (secs / 60) % 60;
    uint64_t hours = (secs / 3600) % 24;
    char clock_str[16];
    snprintf(clock_str, 16, "%02llu:%02llu", hours, mins);
    gui_draw_string(tb->w - 60, tb->y + 13, clock_str, 0x8b949e);

    /* Systray terület */
    gui_draw_rect(tb->w - 90, tb->y + 8, 24, 24, 0x21262d);
    gui_draw_string(tb->w - 84, tb->y + 14, "EN", 0x8b949e);
}

void taskbar_handle_mouse(taskbar_t* tb, int mx, int my, bool click) {
    if (!tb) return;
    if (my < tb->y || my > tb->y + tb->h) { tb->hovered_btn = -1; return; }
    tb->hovered_btn = -1;
    for (uint32_t i = 0; i < tb->btn_count; i++) {
        if (mx >= tb->buttons[i].x && mx <= tb->buttons[i].x + tb->buttons[i].w) {
            tb->hovered_btn = i;
            if (click) { /* Ablak fókusz */ }
            return;
        }
    }
}
