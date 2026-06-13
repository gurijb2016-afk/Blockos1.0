#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
static char g_history[256][512];
static int  g_hist_count=0;
void history_add(const char* cmd) {
    if(g_hist_count<256) strncpy(g_history[g_hist_count++],cmd,511);
}
int history_main(int argc, char** argv) {
    (void)argc;(void)argv;
    for(int i=0;i<g_hist_count;i++){
        char buf[8]; snprintf(buf,8,"%4d  ",i+1);
        terminal_print(buf); terminal_print(g_history[i]); terminal_print("\n");
    }
    return 0;
}
