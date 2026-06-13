#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
#define SORT_MAX 1024
int sort_main(int argc, char** argv) {
    bool reverse=false; const char* file=NULL;
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-r")==0) reverse=true;
        else file=argv[i];
    }
    if (!file) { terminal_print("Használat: sort <fájl>\n"); return 1; }
    int fd=vfs_open(file,VFS_O_RDONLY);
    if (fd<0){terminal_print("sort: nem található\n");return 1;}
    static char lines[SORT_MAX][256]; int count=0; char line[256]; int li=0; uint8_t ch;
    while (vfs_read(fd,&ch,1)>0&&count<SORT_MAX) {
        if (ch=='\n'||li>=255){line[li]=0;strncpy(lines[count++],line,255);li=0;}
        else line[li++]=ch;
    }
    vfs_close(fd);
    /* Bubble sort */
    for (int i=0;i<count-1;i++) for (int j=0;j<count-i-1;j++) {
        int cmp=strcmp(lines[j],lines[j+1]);
        if ((!reverse&&cmp>0)||(reverse&&cmp<0)) {
            char tmp[256]; strcpy(tmp,lines[j]); strcpy(lines[j],lines[j+1]); strcpy(lines[j+1],tmp);
        }
    }
    for (int i=0;i<count;i++){terminal_print(lines[i]);terminal_print("\n");}
    return 0;
}
