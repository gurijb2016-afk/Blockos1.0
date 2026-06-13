#include "gui.h"
#include <string.h>

/* Simple 8x8 font data (basic ASCII) */
static const uint8_t font_data[256][8] = {
    /* Space */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ! */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* " */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* # */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* $ */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* % */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* & */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ' */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ( */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ) */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* * */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* + */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* , */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* - */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* . */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* / */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 0 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 1 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 2 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 3 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 4 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 5 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 6 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 7 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 8 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 9 */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* : */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ; */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* < */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* = */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* > */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* ? */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* @ */    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* A */    {0x18, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x00, 0x00},
    /* B */    {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x7C, 0x00, 0x00},
    /* C */    {0x3C, 0x66, 0x60, 0x60, 0x66, 0x3C, 0x00, 0x00},
    /* D */    {0x78, 0x6C, 0x66, 0x66, 0x6C, 0x78, 0x00, 0x00},
    /* E */    {0x7E, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00, 0x00},
    /* F */    {0x7E, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00, 0x00},
    /* G */    {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x3C, 0x00, 0x00},
    /* H */    {0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00, 0x00},
    /* I */    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00, 0x00},
    /* J */    {0x7E, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00, 0x00},
    /* K */    {0x66, 0x6C, 0x78, 0x78, 0x6C, 0x66, 0x00, 0x00},
    /* L */    {0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00, 0x00},
    /* M */    {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x00, 0x00},
    /* N */    {0x66, 0x76, 0x7E, 0x6E, 0x66, 0x66, 0x00, 0x00},
    /* O */    {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00},
    /* P */    {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x00, 0x00},
    /* Q */    {0x3C, 0x66, 0x66, 0x6E, 0x3C, 0x0E, 0x00, 0x00},
    /* R */    {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x00, 0x00},
    /* S */    {0x3C, 0x60, 0x3C, 0x06, 0x06, 0x3C, 0x00, 0x00},
    /* T */    {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00},
    /* U */    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00},
    /* V */    {0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00},
    /* W */    {0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00, 0x00},
    /* X */    {0x66, 0x66, 0x3C, 0x3C, 0x66, 0x66, 0x00, 0x00},
    /* Y */    {0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00, 0x00},
    /* Z */    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00, 0x00},
};

static uint8_t* framebuffer = (uint8_t*)SCREEN_BUFFER;
static uint8_t backbuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void gui_init(void) { gui_clear(COLOR_BLACK); }

void gui_clear(Color bg_color) { memset(backbuffer, bg_color, SCREEN_WIDTH * SCREEN_HEIGHT); }

void gui_set_pixel(uint16_t x, uint16_t y, Color color) {
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    backbuffer[y * SCREEN_WIDTH + x] = color;
}

void gui_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, Color color) {
    int dx = (x2>x1)?(x2-x1):(x1-x2), dy = (y2>y1)?(y2-y1):(y1-y2);
    int sx = (x1<x2)?1:-1, sy = (y1<y2)?1:-1, err = dx-dy;
    int x=x1, y=y1;
    while(1) {
        gui_set_pixel(x,y,color);
        if(x==x2&&y==y2) break;
        int e2=2*err;
        if(e2>-dy){err-=dy;x+=sx;}
        if(e2<dx){err+=dx;y+=sy;}
    }
}

void gui_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
    gui_draw_line(x,y,x+w,y,color);
    gui_draw_line(x+w,y,x+w,y+h,color);
    gui_draw_line(x+w,y+h,x,y+h,color);
    gui_draw_line(x,y+h,x,y,color);
}

void gui_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
    for(uint16_t yy=y;yy<y+h&&yy<SCREEN_HEIGHT;yy++)
        for(uint16_t xx=x;xx<x+w&&xx<SCREEN_WIDTH;xx++)
            gui_set_pixel(xx,yy,color);
}

void gui_draw_circle(uint16_t x, uint16_t y, uint16_t r, Color color) {
    int p=1-r, px=0, py=r;
    while(px<=py) {
        gui_set_pixel(x+px,y+py,color); gui_set_pixel(x-px,y+py,color);
        gui_set_pixel(x+px,y-py,color); gui_set_pixel(x-px,y-py,color);
        gui_set_pixel(x+py,y+px,color); gui_set_pixel(x-py,y+px,color);
        gui_set_pixel(x+py,y-px,color); gui_set_pixel(x-py,y-px,color);
        if(p<0) p+=2*px+1; else{p+=2*(px-py)+1;py--;} px++;
    }
}

void gui_fill_circle(uint16_t x, uint16_t y, uint16_t r, Color color) {
    for(int py=-r;py<=r;py++){
        int d=r*r-py*py; if(d<0) continue;
        int px=0; while(px*px<=d){gui_set_pixel(x+px,y+py,color);gui_set_pixel(x-px,y+py,color);px++;}
    }
}

void gui_draw_char(uint16_t x, uint16_t y, char c, Color color) {
    const uint8_t* g=font_data[(uint8_t)c];
    for(int yy=0;yy<8;yy++){uint8_t row=g[yy];for(int xx=0;xx<8;xx++) if(row&(1<<(7-xx))) gui_set_pixel(x+xx,y+yy,color);}
}

void gui_draw_string(uint16_t x, uint16_t y, const char* str, Color color) {
    for(int i=0;str[i];i++) gui_draw_char(x+(i*8),y,str[i],color);
}

Button* gui_create_button(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          const char* label, Color color, void (*on_click)(void)) {
    static Button s[16]; static int n=0;
    if(n>=16) return NULL;
    Button* b=&s[n++];
    b->x=x;b->y=y;b->width=w;b->height=h;b->label=label;
    b->color=color;b->text_color=COLOR_WHITE;b->on_click=on_click;b->pressed=0;b->enabled=1;
    return b;
}

void gui_draw_button(Button* b) {
    if(!b) return;
    Color c=b->pressed?COLOR_LIGHTBLUE:b->color; if(!b->enabled) c=COLOR_DARKGRAY;
    gui_fill_rect(b->x,b->y,b->width,b->height,c);
    gui_draw_rect(b->x,b->y,b->width,b->height,COLOR_DARKGRAY);
    if(b->label) gui_draw_string(b->x+5,b->y+(b->height/2)-4,b->label,b->text_color);
}

void gui_handle_button_click(Button* b, uint16_t cx, uint16_t cy) {
    if(!b||!b->enabled) return;
    if(cx>=b->x&&cx<b->x+b->width&&cy>=b->y&&cy<b->y+b->height){b->pressed=1;if(b->on_click)b->on_click();}
    else b->pressed=0;
}

void gui_button_set_enabled(Button* b, bool e){if(b)b->enabled=e;}

TextBox* gui_create_textbox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t max) {
    static TextBox s[8]; static int n=0;
    if(n>=8) return NULL;
    TextBox* b=&s[n++];
    b->x=x;b->y=y;b->width=w;b->height=h;b->bg_color=COLOR_WHITE;b->text_color=COLOR_BLACK;
    b->cursor_pos=0;b->max_length=max;b->focused=0;memset(b->buffer,0,256);
    return b;
}

void gui_draw_textbox(TextBox* b) {
    if(!b) return;
    Color bc=b->focused?COLOR_LIGHTBLUE:COLOR_DARKGRAY;
    gui_fill_rect(b->x,b->y,b->width,b->height,b->bg_color);
    gui_draw_rect(b->x,b->y,b->width,b->height,bc);
    gui_draw_string(b->x+2,b->y+(b->height/2)-4,b->buffer,b->text_color);
    if(b->focused){uint16_t cx=b->x+2+(b->cursor_pos*8);gui_draw_line(cx,b->y+2,cx,b->y+b->height-2,COLOR_BLACK);}
}

void gui_textbox_input_char(TextBox* b, char c){if(b&&b->cursor_pos<b->max_length){b->buffer[b->cursor_pos++]=c;b->buffer[b->cursor_pos]=0;}}
void gui_textbox_backspace(TextBox* b){if(b&&b->cursor_pos>0){b->cursor_pos--;b->buffer[b->cursor_pos]=0;}}
void gui_textbox_set_focused(TextBox* b, bool f){if(b)b->focused=f;}

Label* gui_create_label(uint16_t x, uint16_t y, const char* text, Color color) {
    static Label s[32]; static int n=0;
    if(n>=32) return NULL;
    Label* l=&s[n++]; l->x=x;l->y=y;l->text=text;l->text_color=color; return l;
}

void gui_draw_label(Label* l){if(l)gui_draw_string(l->x,l->y,l->text,l->text_color);}
void gui_label_set_text(Label* l, const char* t){if(l)l->text=t;}

CheckBox* gui_create_checkbox(uint16_t x, uint16_t y, const char* label, Color color) {
    static CheckBox s[16]; static int n=0;
    if(n>=16) return NULL;
    CheckBox* c=&s[n++]; c->x=x;c->y=y;c->label=label;c->color=color;c->text_color=COLOR_WHITE;c->checked=0; return c;
}

void gui_draw_checkbox(CheckBox* c) {
    if(!c) return;
    gui_draw_rect(c->x,c->y,12,12,c->color);
    if(c->checked) gui_fill_rect(c->x+2,c->y+2,8,8,c->color);
    if(c->label) gui_draw_string(c->x+16,c->y+2,c->label,c->text_color);
}

void gui_checkbox_toggle(CheckBox* c){if(c)c->checked=!c->checked;}
void gui_handle_checkbox_click(CheckBox* c, uint16_t cx, uint16_t cy){if(c&&cx>=c->x&&cx<c->x+12&&cy>=c->y&&cy<c->y+12)gui_checkbox_toggle(c);}

ProgressBar* gui_create_progressbar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color bg, Color fill) {
    static ProgressBar s[8]; static int n=0;
    if(n>=8) return NULL;
    ProgressBar* p=&s[n++]; p->x=x;p->y=y;p->width=w;p->height=h;p->bg_color=bg;p->fill_color=fill;p->percentage=0; return p;
}

void gui_draw_progressbar(ProgressBar* p) {
    if(!p) return;
    gui_fill_rect(p->x,p->y,p->width,p->height,p->bg_color);
    gui_fill_rect(p->x,p->y,(p->width*p->percentage)/100,p->height,p->fill_color);
    gui_draw_rect(p->x,p->y,p->width,p->height,COLOR_BLACK);
}

void gui_progressbar_set_value(ProgressBar* p, uint8_t v){if(p&&v<=100)p->percentage=v;}

Window* gui_create_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* title, Color tc) {
    static Window s[4]; static int n=0;
    if(n>=4) return NULL;
    Window* win=&s[n++]; win->x=x;win->y=y;win->width=w;win->height=h;win->title=title;
    win->title_color=tc;win->bg_color=COLOR_LIGHTGRAY;win->visible=1;win->draggable=1; return win;
}

void gui_draw_window(Window* w) {
    if(!w||!w->visible) return;
    gui_fill_rect(w->x,w->y,w->width,20,w->title_color);
    gui_fill_rect(w->x,w->y+20,w->width,w->height-20,w->bg_color);
    gui_draw_rect(w->x,w->y,w->width,w->height,COLOR_BLACK);
    if(w->title) gui_draw_string(w->x+4,w->y+6,w->title,COLOR_WHITE);
}

void gui_window_set_visible(Window* w, bool v){if(w)w->visible=v;}
void gui_window_move(Window* w, uint16_t x, uint16_t y){if(w){w->x=x;w->y=y;}}
void gui_draw_cursor(uint16_t x, uint16_t y, Color c){gui_draw_line(x,y,x+8,y+8,c);gui_draw_line(x,y,x+4,y+4,c);gui_draw_line(x+4,y+4,x+8,y,c);}
void gui_update(void){memcpy(framebuffer,backbuffer,SCREEN_WIDTH*SCREEN_HEIGHT);}
