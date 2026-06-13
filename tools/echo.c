#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
int echo_main(int argc, char** argv) {
    for(int i=1;i<argc;i++){terminal_print(argv[i]);if(i<argc-1)terminal_print(" ");}
    terminal_print("\n"); return 0;
}
