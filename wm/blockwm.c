#include "blockwm.h"
#include "../gui/gui.h"
#include "../drivers/ps2_mouse.h"
#include "../kernel/types.h"
#include <string.h>

/* BlockWM - BlockOS Window Manager / Compositor */

#define MAX_WINDOWS 64
#define TITLEBAR_HEIGHT 24
#define BORDER_SIZE 2
#define MIN_WIDTH 100
#define MIN_HEIGHT 50

/* Window list */
static blockwm_window_t windows[MAX_WINDOWS];
static int window_count = 0;
static int focused_window = -1;
static blockwm_window_t* drag_window = NULL;
static blockwm_window_t* resize_window = NULL;
static int drag_offset_x = 0;
static int drag_offset_y = 0;

/* Compositor framebuffer */
static uint8_t* compositor_buffer = NULL;
static uint32_t screen_width = 0;
static uint32_t screen_height = 0;

/* Colors */
#define COLOR_TITLEBAR_ACTIVE   0x2563EB  // Kék - aktív ablak
#define COLOR_TITLEBAR_INACTIVE 0x1F2937  // Sötét - inaktív
#define COLOR_BORDER_ACTIVE     0x3B82F6  // Világoskék
#define COLOR_BORDER_INACTIVE   0x374151  // Szürke
#define COLOR_CLOSE_BTN         0xEF4444  // Piros
#define COLOR_MIN_BTN           0xF59E0B  // Sárga
#define COLOR_MAX_BTN           0x22C55E  // Zöld
#define COLOR_TEXT              0xFFFFFF  // Fehér
#define COLOR_SHADOW            0x00000044 // Árnyék

/* Initialize BlockWM */
bool blockwm_init(uint32_t width, uint32_t height) {
    screen_width = width;
    screen_height = height;
    window_count = 0;
    focused_window = -1;
    drag_window = NULL;
    resize_window = NULL;

    /* Allocate compositor buffer */
    compositor_buffer = (uint8_t*)malloc(width * height * 4);
    if (!compositor_buffer) return false;

    memset(windows, 0, sizeof(windows));

    return true;
}

/* Create new window */
blockwm_window_t* blockwm_create_window(const char* title, int x, int y,
                                         uint32_t width, uint32_t height,
                                         uint32_t flags) {
    if (window_count >= MAX_WINDOWS) return NULL;

    blockwm_window_t* win = &windows[window_count++];

    win->id = window_count;
    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->flags = flags;
    win->state = BLOCKWM_STATE_NORMAL;
    win->opacity = 255;
    win->z_order = window_count;

    /* Copy title */
    int i = 0;
    while (title[i] && i < 63) {
        win->title[i] = title[i];
        i++;
    }
    win->title[i] = 0;

    /* Allocate window framebuffer */
    win->framebuffer = (uint8_t*)malloc(width * height * 4);
    if (!win->framebuffer) {
        window_count--;
        return NULL;
    }
    memset(win->framebuffer, 0, width * height * 4);

    /* Focus new window */
    focused_window = window_count - 1;

    return win;
}

/* Destroy window */
void blockwm_destroy_window(blockwm_window_t* win) {
    if (!win) return;

    if (win->framebuffer) {
        free(win->framebuffer);
        win->framebuffer = NULL;
    }

    /* Remove from list */
    int idx = win - windows;
    for (int i = idx; i < window_count - 1; i++) {
        windows[i] = windows[i + 1];
    }
    window_count--;

    if (focused_window >= window_count) {
        focused_window = window_count - 1;
    }
}

/* Focus window */
void blockwm_focus_window(blockwm_window_t* win) {
    if (!win) return;
    focused_window = win - windows;

    /* Bring to front */
    int max_z = 0;
    for (int i = 0; i < window_count; i++) {
        if (windows[i].z_order > max_z) max_z = windows[i].z_order;
    }
    win->z_order = max_z + 1;
}

/* Move window */
void blockwm_move_window(blockwm_window_t* win, int x, int y) {
    if (!win) return;

    /* Clamp to screen */
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + win->width > screen_width) x = screen_width - win->width;
    if (y + win->height > screen_height) y = screen_height - win->height;

    win->x = x;
    win->y = y;
}

/* Resize window */
void blockwm_resize_window(blockwm_window_t* win, uint32_t width, uint32_t height) {
    if (!win) return;

    if (width < MIN_WIDTH) width = MIN_WIDTH;
    if (height < MIN_HEIGHT) height = MIN_HEIGHT;

    win->width = width;
    win->height = height;

    /* Reallocate framebuffer */
    if (win->framebuffer) free(win->framebuffer);
    win->framebuffer = (uint8_t*)malloc(width * height * 4);
    if (win->framebuffer) memset(win->framebuffer, 0, width * height * 4);
}

/* Minimize window */
void blockwm_minimize_window(blockwm_window_t* win) {
    if (!win) return;
    win->state = BLOCKWM_STATE_MINIMIZED;
}

/* Maximize window */
void blockwm_maximize_window(blockwm_window_t* win) {
    if (!win) return;

    if (win->state == BLOCKWM_STATE_MAXIMIZED) {
        /* Restore */
        win->x = win->saved_x;
        win->y = win->saved_y;
        win->width = win->saved_width;
        win->height = win->saved_height;
        win->state = BLOCKWM_STATE_NORMAL;
    } else {
        /* Save current position */
        win->saved_x = win->x;
        win->saved_y = win->y;
        win->saved_width = win->width;
        win->saved_height = win->height;

        /* Maximize */
        win->x = 0;
        win->y = TITLEBAR_HEIGHT;
        win->width = screen_width;
        win->height = screen_height - TITLEBAR_HEIGHT;
        win->state = BLOCKWM_STATE_MAXIMIZED;
    }
}

/* Draw titlebar */
static void blockwm_draw_titlebar(blockwm_window_t* win) {
    if (!win) return;

    bool is_focused = (focused_window == (win - windows));
    uint32_t tb_color = is_focused ? COLOR_TITLEBAR_ACTIVE : COLOR_TITLEBAR_INACTIVE;
    uint32_t border_color = is_focused ? COLOR_BORDER_ACTIVE : COLOR_BORDER_INACTIVE;

    /* Draw titlebar background */
    gui_draw_rect(win->x, win->y - TITLEBAR_HEIGHT,
                  win->width, TITLEBAR_HEIGHT, tb_color);

    /* Draw border */
    gui_draw_rect_outline(win->x - BORDER_SIZE,
                          win->y - TITLEBAR_HEIGHT - BORDER_SIZE,
                          win->width + BORDER_SIZE * 2,
                          win->height + TITLEBAR_HEIGHT + BORDER_SIZE * 2,
                          BORDER_SIZE, border_color);

    /* Window title */
    gui_draw_string(win->x + 10,
                    win->y - TITLEBAR_HEIGHT + 5,
                    win->title, COLOR_TEXT);

    /* Close button - piros */
    gui_draw_circle(win->x + win->width - 12,
                    win->y - TITLEBAR_HEIGHT + 12, 6, COLOR_CLOSE_BTN);

    /* Minimize button - sárga */
    gui_draw_circle(win->x + win->width - 30,
                    win->y - TITLEBAR_HEIGHT + 12, 6, COLOR_MIN_BTN);

    /* Maximize button - zöld */
    gui_draw_circle(win->x + win->width - 48,
                    win->y - TITLEBAR_HEIGHT + 12, 6, COLOR_MAX_BTN);
}

/* Draw shadow */
static void blockwm_draw_shadow(blockwm_window_t* win) {
    if (!win) return;

    /* Simple shadow - offset rectangle */
    gui_draw_rect(win->x + 6, win->y + 6,
                  win->width, win->height + TITLEBAR_HEIGHT,
                  0x111111);
}

/* Composite all windows */
void blockwm_composite(void) {
    if (!compositor_buffer) return;

    /* Sort windows by z_order */
    /* Simple bubble sort */
    for (int i = 0; i < window_count - 1; i++) {
        for (int j = 0; j < window_count - i - 1; j++) {
            if (windows[j].z_order > windows[j+1].z_order) {
                blockwm_window_t tmp = windows[j];
                windows[j] = windows[j+1];
                windows[j+1] = tmp;
            }
        }
    }

    /* Draw each window */
    for (int i = 0; i < window_count; i++) {
        blockwm_window_t* win = &windows[i];

        if (win->state == BLOCKWM_STATE_MINIMIZED) continue;

        /* Draw shadow */
        blockwm_draw_shadow(win);

        /* Draw titlebar */
        blockwm_draw_titlebar(win);

        /* Draw window content */
        if (win->framebuffer) {
            gui_blit(win->framebuffer, win->x, win->y,
                     win->width, win->height, win->opacity);
        }
    }
}

/* Handle mouse input */
void blockwm_handle_mouse(int x, int y, bool left, bool right) {

    /* Handle drag */
    if (drag_window && left) {
        blockwm_move_window(drag_window,
                            x - drag_offset_x,
                            y - drag_offset_y);
        return;
    } else {
        drag_window = NULL;
    }

    /* Check window hits */
    for (int i = window_count - 1; i >= 0; i--) {
        blockwm_window_t* win = &windows[i];
        if (win->state == BLOCKWM_STATE_MINIMIZED) continue;

        /* Check titlebar hit */
        if (x >= win->x && x <= win->x + (int)win->width &&
            y >= win->y - TITLEBAR_HEIGHT && y <= win->y) {

            if (left) {
                blockwm_focus_window(win);

                /* Close button */
                if (x >= win->x + (int)win->width - 18 &&
                    x <= win->x + (int)win->width - 6) {
                    blockwm_destroy_window(win);
                    return;
                }

                /* Minimize button */
                if (x >= win->x + (int)win->width - 36 &&
                    x <= win->x + (int)win->width - 24) {
                    blockwm_minimize_window(win);
                    return;
                }

                /* Maximize button */
                if (x >= win->x + (int)win->width - 54 &&
                    x <= win->x + (int)win->width - 42) {
                    blockwm_maximize_window(win);
                    return;
                }

                /* Start drag */
                drag_window = win;
                drag_offset_x = x - win->x;
                drag_offset_y = y - (win->y - TITLEBAR_HEIGHT);
            }
            return;
        }

        /* Check window content hit */
        if (x >= win->x && x <= win->x + (int)win->width &&
            y >= win->y && y <= win->y + (int)win->height) {
            if (left) blockwm_focus_window(win);
            return;
        }
    }
}

/* Get window by ID */
blockwm_window_t* blockwm_get_window(int id) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].id == id) return &windows[i];
    }
    return NULL;
}

/* Get window count */
int blockwm_get_window_count(void) {
    return window_count;
}

/* Get focused window */
blockwm_window_t* blockwm_get_focused(void) {
    if (focused_window < 0 || focused_window >= window_count) return NULL;
    return &windows[focused_window];
}
