#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
int free_main(int argc, char** argv){
    (void)argc;(void)argv;
    extern uint32_t mem_get_total(void); extern uint32_t mem_get_free(void);
    uint32_t tot=mem_get_total(),fr=mem_get_free(),us=tot-fr;
    terminal_print("              total        used        free\n");
    char buf[128]; snprintf(buf,128,"Mem:      %7u     %7u     %7u\n",tot/1024,us/1024,fr/1024);
    terminal_print(buf); return 0;
}
