#include "tools.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);

int tools_dispatch(const char* cmd, int argc, char** argv) {
    for (int i=0; g_tools[i].name; i++) {
        if (strcmp(g_tools[i].name, cmd)==0)
            return g_tools[i].main(argc, argv);
    }

    /* Help */
    if (strcmp(cmd,"help")==0) {
        terminal_print("BlockOS beépített parancsok:\n\n");
        for (int i=0; g_tools[i].name; i++) {
            char buf[64];
            snprintf(buf,64,"  %-12s %s\n", g_tools[i].name, g_tools[i].desc);
            terminal_print(buf);
        }
        return 0;
    }

    terminal_print("Ismeretlen parancs: ");
    terminal_print(cmd);
    terminal_print("\nGépeld 'help' a parancsok listájához.\n");
    return 127;
}
