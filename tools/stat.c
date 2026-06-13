#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int stat_main(int argc, char** argv) {
    if(argc<2){terminal_print("Használat: stat <fájl>\n");return 1;}
    vfs_node_t node;
    if(!vfs_stat(argv[1],&node)){terminal_print("stat: nem található\n");return 1;}
    char buf[256];
    snprintf(buf,256,
        "  Fájl: %s\n"
        " Méret: %u\n"
        "Típus: %s\n",
        node.name, node.size,
        node.type==VFS_TYPE_DIR?"könyvtár":"fájl");
    terminal_print(buf);
    return 0;
}
