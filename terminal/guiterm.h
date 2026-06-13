#ifndef GUITERM_H
#define GUITERM_H
#include "../../kernel/types.h"
#include "../../wm/blockwm.h"
#define TERM_COLS      120
#define TERM_ROWS       35
#define TERM_CHAR_W      8
#define TERM_CHAR_H     16
#define TERM_MAX_SCROLL 1000
#define TERM_HISTORY    256
#define TERM_BG  0x0d1117
#define TERM_FG  0xe0e0e0
#define TERM_GREEN  0x22c55e
#define TERM_BLUE   0x3b82f6
#define TERM_RED    0xef4444
#define TERM_YELLOW 0xf59e0b
#define TERM_CYAN   0x06b6d4
#define TERM_GRAY   0x6b7280
typedef struct { char ch; uint32_t fg,bg; bool bold,underline; } term_cell_t;
typedef struct {
    term_cell_t cells[TERM_MAX_SCROLL][TERM_COLS];
    int rows,cols,cursor_row,cursor_col,scroll_offset,total_rows;
    uint32_t fg,bg; bool bold,underline;
    char input_buf[512]; int input_len;
    char history[TERM_HISTORY][512]; int history_count,history_idx;
    bool cursor_visible; uint32_t blink_timer;
    blockwm_window_t* window;
} guiterm_t;
void guiterm_init(guiterm_t* t);
void guiterm_putchar(guiterm_t* t, char c);
void guiterm_print(guiterm_t* t, const char* s);
void guiterm_print_color(guiterm_t* t, const char* s, uint32_t fg);
void guiterm_clear(guiterm_t* t);
void guiterm_draw(guiterm_t* t);
void guiterm_handle_key(guiterm_t* t, char key);
void guiterm_scroll(guiterm_t* t, int delta);
void guiterm_newline(guiterm_t* t);
void terminal_print(const char* s);
void terminal_print_color(const char* s, uint32_t color);
#endif
