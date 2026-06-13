#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
int uname_main(int argc, char** argv){
    bool all=argc>1&&strcmp(argv[1],"-a")==0;
    if(all) terminal_print("BlockOS 1.0 blockos 1.0.0 #1 SMP x86_64 BlockOS\n");
    else terminal_print("BlockOS\n");
    return 0;
}
