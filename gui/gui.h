#ifndef GUI_H
#define GUI_H

#include "../kernel/types.h"

/* Színek */
#define COLOR_BLACK      0x000000
#define COLOR_WHITE      0xFFFFFF
#define COLOR_RED        0xFF0000
#define COLOR_GREEN      0x00FF00
#define COLOR_BLUE       0x0000FF
#define COLOR_CYAN       0x00FFFF
#define COLOR_YELLOW     0xFFFF00
#define COLOR_GRAY       0x808080
#define COLOR_DARKGRAY   0x404040
#define COLOR_LIGHTGRAY  0xC0C0C0
#define COLOR_BLOCKOS    0x2563EB

#define SCREEN_WIDTH     1920
#define SCREEN_HEIGHT    1080

/* Gomb struktúra */
typedef struct {
    int x, y;
    uint32_t width, height;
    char label[64];
    uint32_t color;
    void (*callback)(void);
    bool hovered;
    bool pressed;
} Button;

/* GUI függvények */
void gui_init(void);
void gui_clear(uint32_t color);
void gui_update(void);
void gui_put_pixel(int x, int y, uint32_t color);
void gui_draw_rect(int x, int y, uint32_t w, uint32_t h, uint32_t color);
void gui_draw_rect_outline(int x, int y, uint32_t w, uint32_t h, uint32_t thickness, uint32_t color);
void gui_draw_circle(int cx, int cy, int r, uint32_t color);
void gui_draw_string(int x, int y, const char* str, uint32_t color);
void gui_draw_char(int x, int y, char c, uint32_t color);
void gui_blit(uint8_t* src, int x, int y, uint32_t w, uint32_t h, uint8_t opacity);
void gui_draw_cursor(int x, int y, uint32_t color);

Button* gui_create_button(int x, int y, uint32_t w, uint32_t h,
                           const char* label, uint32_t color,
                           void (*callback)(void));
void gui_draw_button(Button* btn);
void gui_handle_button_click(Button* btn, int mx, int my);

#endif
