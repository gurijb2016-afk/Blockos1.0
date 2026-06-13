#include "contextmenu.h"
#include "../gui/gui.h"
#include "../kernel/generic.h"

void ctx_init(contextmenu_t* ctx) {
    memset(ctx, 0, sizeof(contextmenu_t));
    ctx->visible = false;
    ctx->hovered = -1;
    ctx->opacity = 0.0f;
}

void ctx_add_item(contextmenu_t* ctx, const char* label, const char* shortcut,
                   void (*cb)(void*), void* ud) {
    if (!ctx || ctx->item_count >= CTX_MAX_ITEMS) return;
    ctx_item_t* item = &ctx->items[ctx->item_count++];
    item->type    = CTX_ITEM;
    item->enabled = true;
    item->checked = false;
    item->callback = cb;
    item->userdata = ud;
    strncpy(item->label, label, 47);
    if (shortcut) strncpy(item->shortcut, shortcut, 11);
}

void ctx_add_separator(contextmenu_t* ctx) {
    if (!ctx || ctx->item_count >= CTX_MAX_ITEMS) return;
    ctx->items[ctx->item_count++].type = CTX_SEPARATOR;
}

void ctx_show(contextmenu_t* ctx, int x, int y) {
    if (!ctx) return;

    /* Méret számítás */
    ctx->w = CTX_MIN_W;
    int real_h = 8;
    for (uint32_t i = 0; i < ctx->item_count; i++)
        real_h += ctx->items[i].type == CTX_SEPARATOR ? 9 : CTX_ITEM_H;
    ctx->h = real_h + 4;

    /* Képernyőn belül marad */
    ctx->x = x;
    ctx->y = y;

    ctx->visible = true;
    ctx->hovered = -1;
    ctx->opacity = 1.0f;
}

void ctx_hide(contextmenu_t* ctx) {
    if (ctx) { ctx->visible = false; ctx->hovered = -1; }
}

void ctx_draw(contextmenu_t* ctx) {
    if (!ctx || !ctx->visible) return;

    /* Árnyék */
    gui_draw_rect(ctx->x + 4, ctx->y + 4, ctx->w, ctx->h, 0x00000066);

    /* Háttér */
    gui_draw_rect(ctx->x, ctx->y, ctx->w, ctx->h, 0x1f2937);
    gui_draw_rect_outline(ctx->x, ctx->y, ctx->w, ctx->h, 1, 0x374151);

    int iy = ctx->y + 4;
    for (uint32_t i = 0; i < ctx->item_count; i++) {
        ctx_item_t* item = &ctx->items[i];

        if (item->type == CTX_SEPARATOR) {
            gui_draw_rect(ctx->x + 8, iy + 4, ctx->w - 16, 1, 0x374151);
            iy += 9;
            continue;
        }

        /* Hover kiemelés */
        if (ctx->hovered == (int)i) {
            gui_draw_rect(ctx->x + 2, iy, ctx->w - 4, CTX_ITEM_H, 0x2563eb);
        }

        uint32_t text_col = item->enabled ?
            (ctx->hovered == (int)i ? 0xffffff : 0xe0e0e0) : 0x4b5563;

        /* Checkmark */
        if (item->checked)
            gui_draw_string(ctx->x + 6, iy + 6, "✓", 0x22c55e);

        /* Label */
        gui_draw_string(ctx->x + 22, iy + 6, item->label, text_col);

        /* Shortcut */
        if (item->shortcut[0])
            gui_draw_string(ctx->x + ctx->w - 50, iy + 6, item->shortcut, 0x6b7280);

        iy += CTX_ITEM_H;
    }
}

void ctx_handle_mouse(contextmenu_t* ctx, int mx, int my, bool click) {
    if (!ctx || !ctx->visible) return;

    /* Kívül klikk → bezár */
    if (click && (mx < ctx->x || mx > ctx->x + ctx->w ||
                  my < ctx->y || my > ctx->y + ctx->h)) {
        ctx_hide(ctx);
        return;
    }

    ctx->hovered = -1;
    int iy = ctx->y + 4;
    for (uint32_t i = 0; i < ctx->item_count; i++) {
        ctx_item_t* item = &ctx->items[i];
        if (item->type == CTX_SEPARATOR) { iy += 9; continue; }
        if (mx >= ctx->x && mx <= ctx->x + ctx->w &&
            my >= iy && my < iy + CTX_ITEM_H) {
            ctx->hovered = i;
            if (click && item->enabled && item->callback) {
                item->callback(item->userdata);
                ctx_hide(ctx);
            }
        }
        iy += CTX_ITEM_H;
    }
}

bool ctx_is_visible(contextmenu_t* ctx) { return ctx && ctx->visible; }
