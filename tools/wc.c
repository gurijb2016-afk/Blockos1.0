#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int wc_main(int argc, char** argv) {
    if (argc<2) { terminal_print("Használat: wc <fájl>\n"); return 1; }
    bool lines=false,words=false,chars=false,all=true;
    const char* file=argv[argc-1];
    for (int i=1;i<argc-1;i++) {
        if (strcmp(argv[i],"-l")==0){lines=true;all=false;}
        if (strcmp(argv[i],"-w")==0){words=true;all=false;}
        if (strcmp(argv[i],"-c")==0){chars=true;all=false;}
    }
    int fd=vfs_open(file,VFS_O_RDONLY);
    if (fd<0){terminal_print("wc: nem található\n");return 1;}
    uint32_t lc=0,wc=0,cc=0; bool in_word=false; uint8_t ch;
    while (vfs_read(fd,&ch,1)>0) {
        cc++;
        if (ch=='\n') lc++;
        if (ch==' '||ch=='\t'||ch=='\n') { if(in_word){wc++;in_word=false;} }
        else in_word=true;
    }
    vfs_close(fd);
    char buf[64];
    if (all||lines) { snprintf(buf,64,"%7u",lc); terminal_print(buf); }
    if (all||words) { snprintf(buf,64,"%7u",wc); terminal_print(buf); }
    if (all||chars) { snprintf(buf,64,"%7u",cc); terminal_print(buf); }
    terminal_print(" "); terminal_print(file); terminal_print("\n");
    return 0;
}
