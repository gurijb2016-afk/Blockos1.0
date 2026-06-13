#ifndef BLOCKFM_H
#define BLOCKFM_H
#include "../../kernel/types.h"
#define FM_MAX_ENTRIES 512
#define FM_MAX_PATH    1024
#define FM_TYPE_DIR    0
#define FM_TYPE_FILE   1
#define FM_TYPE_IMAGE  2
#define FM_TYPE_CODE   3
typedef struct { char name[256]; char path[FM_MAX_PATH]; uint8_t type; uint32_t size; bool selected; } fm_entry_t;
typedef struct { char path[FM_MAX_PATH]; fm_entry_t entries[FM_MAX_ENTRIES]; uint32_t entry_count; int selected_idx; int scroll_offset; } fm_panel_t;
bool fm_init(void);
bool fm_navigate(int panel, const char* path);
bool fm_go_up(int panel);
bool fm_copy(const char* src, const char* dst);
bool fm_delete(const char* path);
bool fm_mkdir(const char* parent, const char* name);
void fm_draw(void);
void fm_handle_mouse(int x, int y, bool click);
#endif
