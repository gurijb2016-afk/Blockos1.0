#include "../kernel/types.h"
#include "../kernel/generic.h"
#include "../kernel/process.h"
#include "../fs/vfs.h"

/* BlockOS Shell */

#define SHELL_MAX_CMD   256
#define SHELL_MAX_ARGS  16
#define SHELL_PROMPT    "blockos$ "

static char cmd_buffer[SHELL_MAX_CMD];
static int  cmd_pos = 0;

/* Beépített parancsok */
static void cmd_help(void) {
    /* Help szöveg */ }
static void cmd_ls(const char* path) {
    /* Könyvtár listázás */ }
static void cmd_cd(const char* path) {
    /* Könyvtár váltás */ }
static void cmd_cat(const char* file) {
    /* Fájl tartalom */ }
static void cmd_mkdir(const char* path) {
    /* Könyvtár létrehozás */ }
static void cmd_clear(void) {
    /* Képernyő törlés */ }
static void cmd_ps(void) {
    /* Folyamat lista */ }
static void cmd_uname(void) {
    /* Rendszer infó */ }
static void cmd_free(void) {
    /* Memória infó */ }
static void cmd_uptime(void) {
    /* Uptime */ }

/* Parancs feldolgozás */
void shell_process_cmd(char* cmd) {
    char* args[SHELL_MAX_ARGS];
    int argc = 0;
    char* token = strtok(cmd, " ");

    while (token && argc < SHELL_MAX_ARGS) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    if (argc == 0) return;

    if      (strcmp(args[0], "help")   == 0) cmd_help();
    else if (strcmp(args[0], "ls")     == 0) cmd_ls(argc > 1 ? args[1] : ".");
    else if (strcmp(args[0], "cd")     == 0) cmd_cd(argc > 1 ? args[1] : "/");
    else if (strcmp(args[0], "cat")    == 0) cmd_cat(argc > 1 ? args[1] : "");
    else if (strcmp(args[0], "mkdir")  == 0) cmd_mkdir(argc > 1 ? args[1] : "");
    else if (strcmp(args[0], "clear")  == 0) cmd_clear();
    else if (strcmp(args[0], "ps")     == 0) cmd_ps();
    else if (strcmp(args[0], "uname")  == 0) cmd_uname();
    else if (strcmp(args[0], "free")   == 0) cmd_free();
    else if (strcmp(args[0], "uptime") == 0) cmd_uptime();
}

/* Shell főciklus */
void shell_main(void) {
    while (1) {
        /* Prompt kiírás */
        /* Billentyű olvasás */
        /* Parancs feldolgozás */
        shell_process_cmd(cmd_buffer);
        cmd_pos = 0;
    }
}
