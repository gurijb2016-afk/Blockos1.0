#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int grep_main(int argc, char** argv){
    if(argc<3){terminal_print("Használat: grep <minta> <fájl>\n");return 1;}
    const char* pat=argv[1]; const char* file=argv[2];
    int fd=vfs_open(file,VFS_O_RDONLY);
    if(fd<0){terminal_print("grep: nem található: ");terminal_print(file);terminal_print("\n");return 1;}
    char line[1024]; int li=0; uint8_t ch;
    while(vfs_read(fd,&ch,1)>0){
        if(ch=='\n'||li>=1023){line[li]=0;li=0;if(strstr(line,pat)){terminal_print(line);terminal_print("\n");}}
        else line[li++]=ch;
    }
    vfs_close(fd); return 0;
}
