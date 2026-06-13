#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int cat_main(int argc, char** argv) {
    if(argc<2){terminal_print("Használat: cat <fájl>\n");return 1;}
    for(int i=1;i<argc;i++){
        int fd=vfs_open(argv[i],VFS_O_RDONLY);
        if(fd<0){terminal_print("cat: nem található: ");terminal_print(argv[i]);terminal_print("\n");continue;}
        uint8_t buf[256]; uint32_t n;
        while((n=vfs_read(fd,buf,sizeof(buf)-1))>0){buf[n]=0;terminal_print((char*)buf);}
        vfs_close(fd);
    }
    return 0;
}
