#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int find_main(int argc, char** argv) {
    const char* path=argc>1?argv[1]:".";
    const char* name=NULL;
    for(int i=2;i<argc-1;i++) if(strcmp(argv[i],"-name")==0) name=argv[i+1];
    terminal_print(path); terminal_print("\n");
    (void)name;
    return 0;
}
