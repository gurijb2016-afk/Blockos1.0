#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int mkdir_main(int argc, char** argv) {
    if(argc<2){terminal_print("Használat: mkdir <könyvtár>\n");return 1;}
    for(int i=1;i<argc;i++) if(!vfs_mkdir(argv[i],0755)){terminal_print("mkdir: hiba: ");terminal_print(argv[i]);terminal_print("\n");}
    return 0;
}
