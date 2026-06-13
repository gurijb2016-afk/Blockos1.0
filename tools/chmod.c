#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
int chmod_main(int argc, char** argv) {
    if(argc<3){terminal_print("Használat: chmod <mód> <fájl>\n");return 1;}
    /* VFS chmod implementáció */
    return 0;
}
