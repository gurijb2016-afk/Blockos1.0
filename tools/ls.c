#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
extern void terminal_print_color(const char*, uint32_t);
int ls_main(int argc, char** argv) {
    const char* path = argc > 1 && argv[1][0] != '-' ? argv[1] : "/";
    bool long_fmt = false, show_all = false;
    for (int i=1;i<argc;i++) if(argv[i][0]=='-'){
        for(int j=1;argv[i][j];j++){if(argv[i][j]=='l')long_fmt=true;if(argv[i][j]=='a')show_all=true;}
    }
    terminal_print("total 0\n");
    (void)path;(void)long_fmt;(void)show_all;
    return 0;
}
