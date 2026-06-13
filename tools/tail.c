#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
#define TAIL_MAX_LINES 512
int tail_main(int argc, char** argv) {
    int lines=10; const char* file=NULL;
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-n")==0&&i+1<argc) lines=atoi(argv[++i]);
        else if (argv[i][0]!='-') file=argv[i];
    }
    if (!file) { terminal_print("Használat: tail [-n N] <fájl>\n"); return 1; }
    int fd=vfs_open(file,VFS_O_RDONLY);
    if (fd<0) { terminal_print("tail: nem található\n"); return 1; }
    char buf[TAIL_MAX_LINES][256]; int count=0; char line[256]; int li=0; uint8_t ch;
    while (vfs_read(fd,&ch,1)>0) {
        if (ch=='\n'||li>=255) {
            line[li]=0; strncpy(buf[count%TAIL_MAX_LINES],line,255);
            count++; li=0;
        } else line[li++]=ch;
    }
    vfs_close(fd);
    int start=count>lines?count-lines:0;
    for (int i=start;i<count;i++) { terminal_print(buf[i%TAIL_MAX_LINES]); terminal_print("\n"); }
    return 0;
}
