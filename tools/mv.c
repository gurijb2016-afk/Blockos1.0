#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int mv_main(int argc, char** argv) {
    if(argc<3){terminal_print("Használat: mv <forrás> <cél>\n");return 1;}
    if(!vfs_rename(argv[1],argv[2])){terminal_print("mv: hiba\n");return 1;}
    return 0;
}
