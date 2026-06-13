#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int touch_main(int argc, char** argv){
    if(argc<2){terminal_print("Használat: touch <fájl>\n");return 1;}
    for(int i=1;i<argc;i++){int fd=vfs_open(argv[i],VFS_O_CREAT|VFS_O_WRONLY);if(fd>=0)vfs_close(fd);}
    return 0;
}
