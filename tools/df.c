#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../fs/vfs.h"
extern void terminal_print(const char*);
int df_main(int argc, char** argv) {
    (void)argc;(void)argv;
    terminal_print("Filesystem      Size    Used    Avail   Use%  Mounted on\n");
    terminal_print("ramfs           32M     2M      30M     6%    /\n");
    terminal_print("procfs          0       0       0       -     /proc\n");
    terminal_print("fat32           512M    100M    412M    19%   /mnt\n");
    return 0;
}
