#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H
#include "../kernel/types.h"

#define CTX_MAX_ITEMS  20
#define CTX_ITEM_H     26
#define CTX_MIN_W      160
#define CTX_SEPARATOR  0
#define CTX_ITEM       1
#define CTX_SUBMENU    2

typedef struct {
    uint8_t type;
    char    label[48];
    char    shortcut[12];
    bool    enabled;
    bool    checked;
    void  (*callback)(void* userdata);
    void*   userdata;
} ctx_item_t;

typedef struct {
    int        x, y, w, h;
    ctx_item_t items[CTX_MAX_ITEMS];
    uint32_t   item_count;
    bool       visible;
    int        hovered;
    float      opacity;
} contextmenu_t;

void ctx_init(contextmenu_t* ctx);
void ctx_add_item(contextmenu_t* ctx, const char* label, const char* shortcut, void (*cb)(void*), void* ud);
void ctx_add_separator(contextmenu_t* ctx);
void ctx_show(contextmenu_t* ctx, int x, int y);
void ctx_hide(contextmenu_t* ctx);
void ctx_draw(contextmenu_t* ctx);
void ctx_handle_mouse(contextmenu_t* ctx, int mx, int my, bool click);
bool ctx_is_visible(contextmenu_t* ctx);
#endif
