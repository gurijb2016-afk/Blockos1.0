#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
static char g_cwd[1024]="/";
int pwd_main(int argc, char** argv){(void)argc;(void)argv;terminal_print(g_cwd);terminal_print("\n");return 0;}
void pwd_set(const char* p){strncpy(g_cwd,p,1023);}
const char* pwd_get(void){return g_cwd;}
