#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
int date_main(int argc, char** argv) {
    (void)argc;(void)argv;
    uint64_t ticks=kernel_get_ticks();
    uint64_t secs=ticks/100;
    uint64_t mins=(secs/60)%60;
    uint64_t hours=(secs/3600)%24;
    uint64_t days=secs/86400;
    char buf[64];
    snprintf(buf,64,"BlockOS Uptime: %llud %02lluh %02llum %02llus\n",days,hours,mins,secs%60);
    terminal_print(buf);
    return 0;
}
