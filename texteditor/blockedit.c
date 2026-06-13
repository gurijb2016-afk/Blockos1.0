#include "blockedit.h"
#include "../../kernel/generic.h"
#include "../../gui/gui.h"
#include "../../wm/blockwm.h"

#define LINE_H  16
#define GUTTER  50

static struct {
    char lines[EDIT_MAX_LINES][EDIT_MAX_LINE_LEN];
    int  line_count;
    int  cursor_line, cursor_col;
    int  scroll_line;
    char filename[256];
    bool modified;
    int  syntax;
    blockwm_window_t* window;
} g_ed;

static const char* c_kw[] = {
    "if","else","while","for","do","return","void","int","char",
    "uint8_t","uint32_t","bool","true","false","static","extern",
    "const","struct","typedef","include","define",NULL
};

bool edit_init(void) {
    memset(&g_ed, 0, sizeof(g_ed));
    g_ed.line_count = 1;
    g_ed.window = blockwm_create_window(
        "BlockEdit - Szövegszerkesztő", 80, 80, 800, 600,
        BLOCKWM_FLAG_TITLEBAR|BLOCKWM_FLAG_RESIZABLE|BLOCKWM_FLAG_SHADOW);
    return g_ed.window != NULL;
}

bool edit_open(const char* path) {
    strncpy(g_ed.filename, path, 255);
    const char* ext = strrchr(path, '.');
    if (ext) {
        ext++;
        if (strcmp(ext,"c")==0||strcmp(ext,"h")==0) g_ed.syntax = SYNTAX_C;
        else if (strcmp(ext,"asm")==0) g_ed.syntax = SYNTAX_ASM;
    }
    g_ed.line_count = 1;
    g_ed.modified   = false;
    return true;
}

bool edit_save(void) {
    if (!g_ed.filename[0]) return false;
    g_ed.modified = false;
    return true;
}

bool edit_save_as(const char* path) {
    strncpy(g_ed.filename, path, 255);
    return edit_save();
}

void edit_insert_char(char c) {
    char* line = g_ed.lines[g_ed.cursor_line];
    int len = strlen(line);
    int col = g_ed.cursor_col;

    if (c == '\n') {
        if (g_ed.line_count >= EDIT_MAX_LINES) return;
        for (int i = g_ed.line_count; i > g_ed.cursor_line+1; i--)
            memcpy(g_ed.lines[i], g_ed.lines[i-1], EDIT_MAX_LINE_LEN);
        memcpy(g_ed.lines[g_ed.cursor_line+1], line+col, len-col+1);
        line[col] = 0;
        g_ed.line_count++;
        g_ed.cursor_line++;
        g_ed.cursor_col = 0;
    } else if (c == '\t') {
        for (int i=0; i<4; i++) edit_insert_char(' ');
        return;
    } else {
        if (len >= EDIT_MAX_LINE_LEN-1) return;
        memmove(line+col+1, line+col, len-col+1);
        line[col] = c;
        g_ed.cursor_col++;
    }
    g_ed.modified = true;
}

void edit_delete_char(void) {
    char* line = g_ed.lines[g_ed.cursor_line];
    int col = g_ed.cursor_col;
    int len = strlen(line);
    if (col > 0) {
        memmove(line+col-1, line+col, len-col+1);
        g_ed.cursor_col--;
    } else if (g_ed.cursor_line > 0) {
        char* prev = g_ed.lines[g_ed.cursor_line-1];
        int pl = strlen(prev);
        strncat(prev, line, EDIT_MAX_LINE_LEN-pl-1);
        for (int i = g_ed.cursor_line; i < g_ed.line_count-1; i++)
            memcpy(g_ed.lines[i], g_ed.lines[i+1], EDIT_MAX_LINE_LEN);
        g_ed.line_count--;
        g_ed.cursor_line--;
        g_ed.cursor_col = pl;
    }
    g_ed.modified = true;
}

void edit_move_cursor(int dl, int dc) {
    g_ed.cursor_line += dl;
    g_ed.cursor_col  += dc;
    if (g_ed.cursor_line < 0) g_ed.cursor_line = 0;
    if (g_ed.cursor_line >= g_ed.line_count) g_ed.cursor_line = g_ed.line_count-1;
    int ll = strlen(g_ed.lines[g_ed.cursor_line]);
    if (g_ed.cursor_col < 0) g_ed.cursor_col = 0;
    if (g_ed.cursor_col > ll) g_ed.cursor_col = ll;
    if (g_ed.cursor_line < g_ed.scroll_line) g_ed.scroll_line = g_ed.cursor_line;
    if (g_ed.cursor_line >= g_ed.scroll_line+30) g_ed.scroll_line = g_ed.cursor_line-29;
}

int edit_find(const char* text, int start) {
    for (int i = start; i < g_ed.line_count; i++)
        if (strstr(g_ed.lines[i], text)) { g_ed.cursor_line = i; return i; }
    return -1;
}

int edit_replace(const char* find, const char* replace) {
    int count = 0;
    for (int i = 0; i < g_ed.line_count; i++)
        if (strstr(g_ed.lines[i], find)) count++;
    return count;
}

void edit_handle_key(char key) {
    switch(key) {
        case '\b': edit_delete_char(); break;
        case '\n': edit_insert_char('\n'); break;
        default:   if (key >= 32) edit_insert_char(key); break;
    }
}

void edit_draw(void) {
    int W=800, H=600;
    int visible = (H-48)/LINE_H;
    gui_draw_rect(0, 0, W, H, 0x0d1117);
    gui_draw_rect(0, 0, W, 26, 0x161b22);
    gui_draw_string(8,  8, "Új",          0x8b949e);
    gui_draw_string(36, 8, "Megnyit",     0x8b949e);
    gui_draw_string(90, 8, "Mentés",      0x8b949e);
    gui_draw_string(145,8, "Keresés",     0x8b949e);
    if (g_ed.modified) gui_draw_string(W-90, 8, "● Módosítva", 0xf59e0b);
    gui_draw_rect(0, 26, GUTTER, H-46, 0x161b22);

    for (int i = 0; i < visible; i++) {
        int li = g_ed.scroll_line + i;
        if (li >= g_ed.line_count) break;
        int y = 26 + i*LINE_H;
        if (li == g_ed.cursor_line) gui_draw_rect(GUTTER, y, W-GUTTER, LINE_H, 0x161b22);
        char num[8]; snprintf(num, 8, "%4d", li+1);
        gui_draw_string(2, y+2, num, li==g_ed.cursor_line ? 0xe0e0e0 : 0x4b5563);
        gui_draw_string(GUTTER+4, y+2, g_ed.lines[li], 0xe0e0e0);
        if (li == g_ed.cursor_line)
            gui_draw_rect(GUTTER+4+g_ed.cursor_col*8, y, 2, LINE_H, 0xe0e0e0);
    }

    gui_draw_rect(0, H-20, W, 20, 0x161b22);
    char st[128];
    snprintf(st, 128, " Sor:%d/%d  Oszlop:%d  %s  %s",
        g_ed.cursor_line+1, g_ed.line_count, g_ed.cursor_col+1,
        g_ed.filename[0] ? g_ed.filename : "Névtelen",
        g_ed.syntax==SYNTAX_C?"C":g_ed.syntax==SYNTAX_ASM?"ASM":"Szöveg");
    gui_draw_string(4, H-16, st, 0x8b949e);
}
