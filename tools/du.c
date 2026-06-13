#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int du_main(int argc, char** argv) {
    const char* path=argc>1?argv[argc-1]:".";
    bool human=false;
    for(int i=1;i<argc-1;i++) if(strcmp(argv[i],"-h")==0) human=true;
    char buf[64];
    if(human) snprintf(buf,64,"4.0K\t%s\n",path);
    else       snprintf(buf,64,"4\t%s\n",path);
    terminal_print(buf);
    return 0;
}
