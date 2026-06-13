#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int rm_main(int argc, char** argv) {
    if(argc<2){terminal_print("Használat: rm <fájl>\n");return 1;}
    bool r=strcmp(argv[1],"-r")==0||strcmp(argv[1],"-rf")==0;
    int s=r?2:1;
    for(int i=s;i<argc;i++){
        if(!vfs_unlink(argv[i])&&r) vfs_rmdir(argv[i]);
    }
    return 0;
}
