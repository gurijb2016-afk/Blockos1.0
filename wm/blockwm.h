#ifndef BLOCKWM_H
#define BLOCKWM_H

#include "../kernel/types.h"

/* Window states */
#define BLOCKWM_STATE_NORMAL    0
#define BLOCKWM_STATE_MINIMIZED 1
#define BLOCKWM_STATE_MAXIMIZED 2

/* Window flags */
#define BLOCKWM_FLAG_RESIZABLE  (1 << 0)
#define BLOCKWM_FLAG_TITLEBAR   (1 << 1)
#define BLOCKWM_FLAG_SHADOW     (1 << 2)
#define BLOCKWM_FLAG_TOPMOST    (1 << 3)

/* Window structure */
typedef struct {
    int id;
    char title[64];
    int x, y;
    uint32_t width, height;
    uint32_t flags;
    int state;
    uint8_t opacity;
    int z_order;
    uint8_t* framebuffer;

    /* Saved position for maximize/restore */
    int saved_x, saved_y;
    uint32_t saved_width, saved_height;
} blockwm_window_t;

/* BlockWM functions */
bool blockwm_init(uint32_t width, uint32_t height);
blockwm_window_t* blockwm_create_window(const char* title, int x, int y,
                                         uint32_t width, uint32_t height,
                                         uint32_t flags);
void blockwm_destroy_window(blockwm_window_t* win);
void blockwm_focus_window(blockwm_window_t* win);
void blockwm_move_window(blockwm_window_t* win, int x, int y);
void blockwm_resize_window(blockwm_window_t* win, uint32_t width, uint32_t height);
void blockwm_minimize_window(blockwm_window_t* win);
void blockwm_maximize_window(blockwm_window_t* win);
void blockwm_composite(void);
void blockwm_handle_mouse(int x, int y, bool left, bool right);
blockwm_window_t* blockwm_get_window(int id);
blockwm_window_t* blockwm_get_focused(void);
int blockwm_get_window_count(void);

#endif
