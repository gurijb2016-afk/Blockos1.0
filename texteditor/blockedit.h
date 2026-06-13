#ifndef BLOCKEDIT_H
#define BLOCKEDIT_H
#include "../../kernel/types.h"
#define EDIT_MAX_LINES    4096
#define EDIT_MAX_LINE_LEN 1024
#define SYNTAX_NONE  0
#define SYNTAX_C     1
#define SYNTAX_ASM   2
bool edit_init(void);
bool edit_open(const char* path);
bool edit_save(void);
bool edit_save_as(const char* path);
void edit_insert_char(char c);
void edit_delete_char(void);
void edit_move_cursor(int dline, int dcol);
int  edit_find(const char* text, int start_line);
int  edit_replace(const char* find, const char* replace);
void edit_draw(void);
void edit_handle_key(char key);
#endif
