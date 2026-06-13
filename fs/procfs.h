#ifndef PROCFS_H
#define PROCFS_H

#include "../kernel/types.h"
#include "../kernel/process.h"

/* ProcFS konfig */
#define PROCFS_MAX_ENTRIES  256
#define PROCFS_MAX_NAME     64
#define PROCFS_MAX_PATH     256
#define PROCFS_BUFFER_SIZE  4096

/* ProcFS entry típusok */
#define PROCFS_TYPE_DIR     0
#define PROCFS_TYPE_FILE    1
#define PROCFS_TYPE_LINK    2

/* ProcFS entry struktúra */
typedef struct procfs_entry {
    char name[PROCFS_MAX_NAME];
    uint8_t type;
    uint32_t pid;           // 0 = globális
    char* (*read_fn)(uint32_t pid);  // Olvasó callback
    bool (*write_fn)(uint32_t pid, const char* data, uint32_t size);
    struct procfs_entry* children;
    struct procfs_entry* next;
} procfs_entry_t;

/* ProcFS fő struktúra */
typedef struct {
    bool mounted;
    procfs_entry_t* root;
    char mount_point[PROCFS_MAX_PATH];
} procfs_t;

/* Fájl leíró */
typedef struct {
    procfs_entry_t* entry;
    uint32_t pid;
    uint32_t position;
    char* buffer;
    uint32_t buffer_size;
} procfs_file_t;

/* Függvények */
bool procfs_init(procfs_t* fs);
bool procfs_mount(procfs_t* fs, const char* mount_point);
bool procfs_umount(procfs_t* fs);
procfs_file_t* procfs_open(procfs_t* fs, const char* path);
uint32_t procfs_read(procfs_file_t* file, uint8_t* buffer, uint32_t size);
bool procfs_write(procfs_file_t* file, const uint8_t* buffer, uint32_t size);
bool procfs_close(procfs_file_t* file);
bool procfs_list_dir(procfs_t* fs, const char* path, char** entries, uint32_t* count);
void procfs_update(procfs_t* fs);

#endif
