#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int head_main(int argc, char** argv) {
    int lines = 10;
    const char* file = NULL;
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-n")==0 && i+1<argc) { lines=atoi(argv[++i]); }
        else if (argv[i][0]=='-' && argv[i][1]=='n') { lines=atoi(argv[i]+2); }
        else file=argv[i];
    }
    if (!file) { terminal_print("Használat: head [-n N] <fájl>\n"); return 1; }
    int fd=vfs_open(file,VFS_O_RDONLY);
    if (fd<0) { terminal_print("head: nem található\n"); return 1; }
    char line[1024]; int li=0, lnum=0; uint8_t ch;
    while (vfs_read(fd,&ch,1)>0 && lnum<lines) {
        if (ch=='\n'||li>=1023) {
            line[li]=0; terminal_print(line); terminal_print("\n");
            li=0; lnum++;
        } else line[li++]=ch;
    }
    vfs_close(fd); return 0;
}
