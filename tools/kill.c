#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../kernel/process.h"
extern void terminal_print(const char*);
int kill_main(int argc, char** argv){
    if(argc<2){terminal_print("Használat: kill <pid>\n");return 1;}
    uint32_t pid=(uint32_t)atoi(argv[1]);
    process_t* p=process_get_by_pid(pid);
    if(!p){terminal_print("kill: nincs ilyen folyamat\n");return 1;}
    process_exit(0); return 0;
}
