#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int cp_main(int argc, char** argv) {
    if(argc<3){terminal_print("Használat: cp <forrás> <cél>\n");return 1;}
    int fd_src=vfs_open(argv[1],VFS_O_RDONLY);
    if(fd_src<0){terminal_print("cp: nem található: ");terminal_print(argv[1]);terminal_print("\n");return 1;}
    int fd_dst=vfs_open(argv[2],VFS_O_WRONLY|VFS_O_CREAT|VFS_O_TRUNC);
    if(fd_dst<0){vfs_close(fd_src);terminal_print("cp: hiba\n");return 1;}
    uint8_t buf[4096]; uint32_t n;
    while((n=vfs_read(fd_src,buf,sizeof(buf)))>0) vfs_write(fd_dst,buf,n);
    vfs_close(fd_src); vfs_close(fd_dst);
    return 0;
}
