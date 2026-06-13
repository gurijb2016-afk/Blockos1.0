#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
extern void guiterm_clear(void* t);
int clear_main(int argc, char** argv) {
    (void)argc;(void)argv;
    /* ANSI clear */
    terminal_print("\033[2J\033[H");
    return 0;
}
