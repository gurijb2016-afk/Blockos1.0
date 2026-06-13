#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../kernel/types.h"
#define KEY_ESC    0x01
#define KEY_ENTER  0x1C
#define KEY_LSHIFT 0x2A
#define KEY_RSHIFT 0x36
#define KEY_CTRL   0x1D
#define KEY_ALT    0x38
#define KEY_CAPS   0x3A
#define KEY_UP     0x48
#define KEY_DOWN   0x50
#define KEY_LEFT   0x4B
#define KEY_RIGHT  0x4D
#define KEY_BACK   0x0E
#define KB_BUFFER_SIZE 256
typedef struct {
    char     buffer[KB_BUFFER_SIZE];
    uint32_t head, tail;
    bool     shift, ctrl, alt, caps;
} keyboard_t;
void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_input(void);
void keyboard_irq_handler(void);
bool keyboard_is_shift(void);
bool keyboard_is_ctrl(void);
bool keyboard_is_alt(void);
#endif
