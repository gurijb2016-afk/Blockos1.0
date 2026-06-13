#include "guiterm.h"
#include "../../gui/gui.h"
#include "../../kernel/generic.h"

static guiterm_t* g_term = NULL;

void guiterm_init(guiterm_t* t) {
    memset(t, 0, sizeof(guiterm_t));
    t->rows=TERM_ROWS; t->cols=TERM_COLS;
    t->fg=TERM_FG; t->bg=TERM_BG;
    t->cursor_visible=true; t->history_idx=-1;
    t->window=blockwm_create_window("BlockOS Terminal",50,50,
        TERM_COLS*TERM_CHAR_W+20, TERM_ROWS*TERM_CHAR_H+50,
        BLOCKWM_FLAG_TITLEBAR|BLOCKWM_FLAG_RESIZABLE|BLOCKWM_FLAG_SHADOW);
    g_term=t;
    guiterm_print_color(t,"BlockOS Terminal v1.0\n",TERM_GREEN);
    guiterm_print_color(t,"Next Generation of Coders\n\n",TERM_GRAY);
    guiterm_print_color(t,"root@blockos",TERM_GREEN);
    guiterm_print(t,":~$ ");
}

void guiterm_newline(guiterm_t* t) {
    t->cursor_col=0; t->cursor_row++;
    if(t->cursor_row>=TERM_MAX_SCROLL) t->cursor_row=TERM_MAX_SCROLL-1;
    if(t->cursor_row>t->total_rows) t->total_rows=t->cursor_row;
    if(t->cursor_row>=t->scroll_offset+t->rows) t->scroll_offset=t->cursor_row-t->rows+1;
}

void guiterm_putchar(guiterm_t* t, char c) {
    if(c=='\n'){guiterm_newline(t);return;}
    if(c=='\r'){t->cursor_col=0;return;}
    if(c=='\t'){t->cursor_col=(t->cursor_col+8)&~7;return;}
    if(c=='\b'){if(t->cursor_col>0)t->cursor_col--;return;}
    if(t->cursor_row<TERM_MAX_SCROLL&&t->cursor_col<TERM_COLS){
        term_cell_t* cell=&t->cells[t->cursor_row][t->cursor_col];
        cell->ch=c; cell->fg=t->fg; cell->bg=t->bg;
        cell->bold=t->bold; cell->underline=t->underline;
        t->cursor_col++;
        if(t->cursor_col>=TERM_COLS) guiterm_newline(t);
    }
}

void guiterm_print(guiterm_t* t, const char* s){while(*s)guiterm_putchar(t,*s++);}

void guiterm_print_color(guiterm_t* t, const char* s, uint32_t fg){
    uint32_t old=t->fg; t->fg=fg; guiterm_print(t,s); t->fg=old;
}

void guiterm_clear(guiterm_t* t){
    memset(t->cells,0,sizeof(t->cells));
    t->cursor_row=t->cursor_col=t->total_rows=t->scroll_offset=0;
}

void guiterm_scroll(guiterm_t* t, int delta){
    t->scroll_offset+=delta;
    if(t->scroll_offset<0) t->scroll_offset=0;
    int max=t->total_rows-t->rows; if(max<0)max=0;
    if(t->scroll_offset>max) t->scroll_offset=max;
}

void guiterm_draw(guiterm_t* t) {
    if(!t||!t->window) return;
    int W=TERM_COLS*TERM_CHAR_W+20, H=TERM_ROWS*TERM_CHAR_H+50;
    gui_draw_rect(0,0,W,H,TERM_BG);
    gui_draw_rect(0,0,W,28,0x161b22);
    gui_draw_string(8,8,"Fájl",0x8b949e);
    gui_draw_string(44,8,"Szerkesztés",0x8b949e);
    gui_draw_string(120,8,"Nézet",0x8b949e);

    for(int r=t->scroll_offset;r<t->scroll_offset+t->rows&&r<TERM_MAX_SCROLL;r++){
        int y=28+(r-t->scroll_offset)*TERM_CHAR_H;
        for(int c=0;c<t->cols;c++){
            term_cell_t* cell=&t->cells[r][c];
            if(!cell->ch) continue;
            int x=10+c*TERM_CHAR_W;
            if(cell->bg!=TERM_BG) gui_draw_rect(x,y,TERM_CHAR_W,TERM_CHAR_H,cell->bg);
            char ch[2]={cell->ch,0};
            gui_draw_string(x,y+2,ch,cell->fg);
            if(cell->underline) gui_draw_rect(x,y+TERM_CHAR_H-2,TERM_CHAR_W,1,cell->fg);
        }
    }

    /* Kurzor */
    t->blink_timer++;
    if((t->blink_timer/30)%2==0){
        int cx=10+t->cursor_col*TERM_CHAR_W;
        int cy=28+(t->cursor_row-t->scroll_offset)*TERM_CHAR_H;
        gui_draw_rect(cx,cy,TERM_CHAR_W,TERM_CHAR_H,0xe0e0e066);
    }

    /* Input sor */
    gui_draw_rect(0,H-22,W,22,0x161b22);
    gui_draw_rect(0,H-22,W,1,0x21262d);
    char inp[520]; snprintf(inp,520,"$ %s_",t->input_buf);
    gui_draw_string(8,H-16,inp,0x60a5fa);

    /* Scrollbar */
    if(t->total_rows>t->rows){
        int sb_h=H-50;
        int th=sb_h*t->rows/t->total_rows; if(th<20)th=20;
        int ty=28+sb_h*t->scroll_offset/t->total_rows;
        gui_draw_rect(W-8,28,8,sb_h,0x161b22);
        gui_draw_rect(W-7,ty,6,th,0x374151);
    }
}

void guiterm_handle_key(guiterm_t* t, char key) {
    if(!t) return;
    if(key=='\n'){
        guiterm_putchar(t,'\n');
        if(t->input_len>0){
            if(t->history_count<TERM_HISTORY)
                strncpy(t->history[t->history_count++],t->input_buf,511);
            t->history_idx=t->history_count;
            extern void shell_process_cmd(char*);
            shell_process_cmd(t->input_buf);
            t->input_buf[0]=0; t->input_len=0;
        }
        guiterm_print_color(t,"root@blockos",TERM_GREEN);
        guiterm_print(t,":~$ ");
    } else if(key=='\b'){
        if(t->input_len>0){t->input_buf[--t->input_len]=0;guiterm_putchar(t,'\b');}
    } else if(key>=32&&key<127){
        if(t->input_len<511){t->input_buf[t->input_len++]=key;t->input_buf[t->input_len]=0;guiterm_putchar(t,key);}
    }
}

void terminal_print(const char* s){if(g_term)guiterm_print(g_term,s);}
void terminal_print_color(const char* s, uint32_t c){if(g_term)guiterm_print_color(g_term,s,c);}
