#include "types.h"
#include "process.h"
#include "syscall.h"
#include "../fs/ramfs.h"
#include "../fs/procfs.h"
#include "../fs/vfs.h"
#include "../drivers/ps2_mouse.h"
#include "../gui/gui.h"
#include "../wm/blockwm.h"
#include "../display/cursor.h"

/* BlockOS globális rendszer struktúrák */
static ramfs_t  g_ramfs;
static procfs_t g_procfs;

/* Kernel belépési pont */
void kernel_main(void) {

    /* 1. Alap inicializálás */
    process_init();
    syscall_init();

    /* 2. Memória fájlrendszer */
    ramfs_init(&g_ramfs);
    ramfs_mount(&g_ramfs, "/");
    ramfs_create_default_structure(&g_ramfs);

    /* 3. ProcFS */
    procfs_init(&g_procfs);
    procfs_mount(&g_procfs, "/proc");

    /* 4. VFS */
    vfs_init();
    vfs_mount_fat32(0);

    /* 5. Driverek */
    ps2_mouse_init();

    /* 6. GUI + BlockWM */
    gui_init();
    blockwm_init(1920, 1080);
    cursor_manager_init();

    /* 7. Shell indítás */
    process_t* shell = process_create("blocksh");
    if (shell) {
        process_add_to_ready(shell);
        process_set_current(shell);
    }

    /* 8. Fő esemény hurok */
    kernel_loop();
}

/* Fő kernel loop */
void kernel_loop(void) {
    while (1) {
        /* Egér kezelés */
        MouseData mouse = ps2_mouse_get_state();

        /* BlockWM egér esemény */
        blockwm_handle_mouse(
            mouse.x_movement,
            mouse.y_movement,
            mouse.left_button,
            mouse.right_button
        );

        /* Cursor frissítés */
        cursor_manager_set_position(
            mouse.x_movement,
            mouse.y_movement
        );

        /* ProcFS frissítés */
        procfs_update(&g_procfs);

        /* Compositor */
        blockwm_composite();

        /* Cursor rajzolás */
        extern uint8_t* g_framebuffer;
        cursor_manager_draw(g_framebuffer, 1920, 1080);

        /* Display frissítés */
        gui_update();

        /* Kis várakozás */
        for (volatile int i = 0; i < 1000; i++);
    }
}
