#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int ln_main(int argc, char** argv) {
    if(argc<3){terminal_print("Használat: ln [-s] <forrás> <link>\n");return 1;}
    bool symbolic=strcmp(argv[1],"-s")==0;
    const char* src=symbolic?argv[2]:argv[1];
    const char* dst=symbolic?argv[3]:argv[2];
    (void)src;(void)dst;(void)symbolic;
    return 0;
}
