#include "keyboard.h"
#include "../kernel/generic.h"
static keyboard_t g_kb;
static const char sc_map[] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '};
static const char sc_shift[] = {
    0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '};
void keyboard_init(void) {
    memset(&g_kb, 0, sizeof(g_kb));
    while (inb(0x64) & 1) inb(0x60);
}
void keyboard_irq_handler(void) {
    uint8_t sc = inb(0x60);
    bool rel = sc & 0x80; sc &= 0x7F;
    if (sc==KEY_LSHIFT||sc==KEY_RSHIFT){g_kb.shift=!rel;return;}
    if (sc==KEY_CTRL){g_kb.ctrl=!rel;return;}
    if (sc==KEY_ALT){g_kb.alt=!rel;return;}
    if (sc==KEY_CAPS&&!rel){g_kb.caps=!g_kb.caps;return;}
    if (rel) return;
    bool up = g_kb.shift ^ g_kb.caps;
    char c = sc < sizeof(sc_map) ? (up ? sc_shift[sc] : sc_map[sc]) : 0;
    if (!c) return;
    uint32_t next = (g_kb.tail+1)%KB_BUFFER_SIZE;
    if (next != g_kb.head){g_kb.buffer[g_kb.tail]=c;g_kb.tail=next;}
}
bool keyboard_has_input(void){return g_kb.head!=g_kb.tail;}
char keyboard_getchar(void){
    while(!keyboard_has_input());
    char c=g_kb.buffer[g_kb.head];
    g_kb.head=(g_kb.head+1)%KB_BUFFER_SIZE;
    return c;
}
bool keyboard_is_shift(void){return g_kb.shift;}
bool keyboard_is_ctrl(void){return g_kb.ctrl;}
bool keyboard_is_alt(void){return g_kb.alt;}
