#include "blockfm.h"
#include "../../kernel/generic.h"
#include "../../gui/gui.h"
#include "../../wm/blockwm.h"

/* BlockFM - BlockOS Fájlkezelő */

#define FM_ITEM_HEIGHT 22
#define FM_PANEL_W     400

static fm_panel_t g_panels[2];
static int g_active_panel = 0;
static char g_clipboard[FM_MAX_PATH];
static bool g_clipboard_cut = false;
static blockwm_window_t* g_window = NULL;

static const char* fm_get_icon(uint8_t type) {
    switch(type) {
        case FM_TYPE_DIR:   return "[DIR]";
        case FM_TYPE_IMAGE: return "[IMG]";
        case FM_TYPE_CODE:  return "[COD]";
        default:            return "[FIL]";
    }
}

static uint8_t fm_get_type(const char* name) {
    const char* ext = strrchr(name, '.');
    if (!ext) return FM_TYPE_FILE;
    ext++;
    if (strcmp(ext,"png")==0||strcmp(ext,"jpg")==0) return FM_TYPE_IMAGE;
    if (strcmp(ext,"c")==0||strcmp(ext,"h")==0||strcmp(ext,"asm")==0) return FM_TYPE_CODE;
    return FM_TYPE_FILE;
}

bool fm_init(void) {
    memset(g_panels, 0, sizeof(g_panels));
    strcpy(g_panels[0].path, "/");
    strcpy(g_panels[1].path, "/home");
    g_active_panel = 0;

    g_window = blockwm_create_window(
        "BlockFM - Fájlkezelő", 100, 100, 800, 500,
        BLOCKWM_FLAG_TITLEBAR|BLOCKWM_FLAG_RESIZABLE|BLOCKWM_FLAG_SHADOW);

    fm_refresh_panel(0);
    fm_refresh_panel(1);
    return true;
}

bool fm_refresh_panel(int p) {
    if (p < 0 || p > 1) return false;
    g_panels[p].entry_count = 0;
    /* VFS könyvtár olvasás itt lenne */
    return true;
}

bool fm_navigate(int p, const char* path) {
    if (p < 0 || p > 1) return false;
    strncpy(g_panels[p].path, path, FM_MAX_PATH-1);
    return fm_refresh_panel(p);
}

bool fm_go_up(int p) {
    char* last = strrchr(g_panels[p].path, '/');
    if (!last || last == g_panels[p].path) return false;
    *last = 0;
    return fm_refresh_panel(p);
}

bool fm_copy(const char* src, const char* dst) {
    strncpy(g_clipboard, src, FM_MAX_PATH-1);
    g_clipboard_cut = false;
    return true;
}

bool fm_delete(const char* path) { return true; }

bool fm_mkdir(const char* parent, const char* name) { return true; }

void fm_draw(void) {
    gui_draw_rect(0, 0, 800, 500, 0x0d1117);

    /* Toolbar */
    gui_draw_rect(0, 0, 800, 28, 0x161b22);
    gui_draw_string(8,  8, "←",       0x8b949e);
    gui_draw_string(28, 8, "→",       0x8b949e);
    gui_draw_string(50, 8, "↑",       0x8b949e);
    gui_draw_string(80, 8, "Frissít", 0x8b949e);
    gui_draw_string(150,8, "+ Mappa", 0x8b949e);
    gui_draw_string(220,8, "Törlés",  0x8b949e);
    gui_draw_string(280,8, "Másolás", 0x8b949e);
    gui_draw_string(350,8, "Beillesztés", 0x8b949e);

    /* Két panel */
    for (int p = 0; p < 2; p++) {
        int px = p == 0 ? 0 : 400;
        bool active = (g_active_panel == p);

        gui_draw_rect(px, 28, FM_PANEL_W, 444, active ? 0x161b22 : 0x111827);
        gui_draw_rect(px, 28, FM_PANEL_W, 22, 0x21262d);
        gui_draw_string(px+6, 34, g_panels[p].path, 0x60a5fa);
        gui_draw_rect(px, 50, FM_PANEL_W, 18, 0x1f2937);
        gui_draw_string(px+6,  53, "Név",    0x8b949e);
        gui_draw_string(px+300,53, "Méret",  0x8b949e);

        for (uint32_t i = 0; i < g_panels[p].entry_count; i++) {
            fm_entry_t* e = &g_panels[p].entries[i];
            int ey = 68 + i * FM_ITEM_HEIGHT;
            if ((int)i == g_panels[p].selected_idx)
                gui_draw_rect(px, ey, FM_PANEL_W, FM_ITEM_HEIGHT, 0x2563eb33);
            gui_draw_string(px+6,  ey+3, fm_get_icon(e->type), 0x60a5fa);
            gui_draw_string(px+54, ey+3, e->name,
                e->type == FM_TYPE_DIR ? 0x60a5fa : 0xe0e0e0);
        }

        char st[32];
        snprintf(st, 32, "%u elem", g_panels[p].entry_count);
        gui_draw_rect(px, 460, FM_PANEL_W, 18, 0x21262d);
        gui_draw_string(px+6, 463, st, 0x8b949e);
    }
    gui_draw_rect(399, 28, 2, 450, 0x30363d);
}

void fm_handle_mouse(int x, int y, bool click) {
    if (!click) return;
    int p = x < 400 ? 0 : 1;
    g_active_panel = p;
    int idx = (y - 68) / FM_ITEM_HEIGHT;
    if (idx >= 0 && idx < (int)g_panels[p].entry_count)
        g_panels[p].selected_idx = idx;
}
