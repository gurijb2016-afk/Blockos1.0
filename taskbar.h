#ifndef TASKBAR_H
#define TASKBAR_H
#include "../kernel/types.h"
#include "../wm/blockwm.h"

#define TASKBAR_HEIGHT   40
#define TASKBAR_MAX_BTNS 32
#define TASKBAR_POSITION_BOTTOM 0
#define TASKBAR_POSITION_TOP    1

typedef struct {
    char  label[32];
    int   window_id;
    bool  active;
    bool  hovered;
    int   x, w;
} taskbar_btn_t;

typedef struct {
    int           x, y, w, h;
    uint8_t       position;
    taskbar_btn_t buttons[TASKBAR_MAX_BTNS];
    uint32_t      btn_count;
    bool          clock_visible;
    bool          systray_visible;
    int           hovered_btn;
} taskbar_t;

void taskbar_init(taskbar_t* tb, uint32_t screen_w, uint32_t screen_h, uint8_t pos);
void taskbar_add_window(taskbar_t* tb, blockwm_window_t* win);
void taskbar_remove_window(taskbar_t* tb, int window_id);
void taskbar_draw(taskbar_t* tb);
void taskbar_handle_mouse(taskbar_t* tb, int mx, int my, bool click);
void taskbar_update_clock(taskbar_t* tb);
#endif
