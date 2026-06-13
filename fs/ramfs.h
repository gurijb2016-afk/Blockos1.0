#ifndef RAMFS_H
#define RAMFS_H

#include "../kernel/types.h"

/* RAMFS konfig */
#define RAMFS_MAX_FILES     512
#define RAMFS_MAX_DIRS      128
#define RAMFS_MAX_NAME      256
#define RAMFS_MAX_PATH      1024
#define RAMFS_BLOCK_SIZE    4096
#define RAMFS_MAX_BLOCKS    8192   // 8192 * 4096 = 32MB max

/* Fájl típusok */
#define RAMFS_TYPE_FILE     0
#define RAMFS_TYPE_DIR      1
#define RAMFS_TYPE_SYMLINK  2
#define RAMFS_TYPE_DEVICE   3

/* Jogosultságok */
#define RAMFS_PERM_READ     0x04
#define RAMFS_PERM_WRITE    0x02
#define RAMFS_PERM_EXEC     0x01

/* RAMFS inode */
typedef struct ramfs_inode {
    uint32_t id;
    char name[RAMFS_MAX_NAME];
    uint8_t type;
    uint32_t size;
    uint32_t permissions;
    uint32_t uid;
    uint32_t gid;
    uint64_t created;
    uint64_t modified;
    uint8_t* data;
    uint32_t data_size;
    struct ramfs_inode* parent;
    struct ramfs_inode* children;
    struct ramfs_inode* next;
    char symlink_target[RAMFS_MAX_PATH];
} ramfs_inode_t;

/* RAMFS fájl leíró */
typedef struct {
    ramfs_inode_t* inode;
    uint32_t position;
    uint32_t flags;
} ramfs_file_t;

/* RAMFS fő struktúra */
typedef struct {
    bool mounted;
    ramfs_inode_t* root;
    uint32_t total_size;
    uint32_t used_size;
    uint32_t file_count;
    uint32_t dir_count;
    char mount_point[RAMFS_MAX_PATH];
} ramfs_t;

/* Függvények */
bool ramfs_init(ramfs_t* fs);
bool ramfs_mount(ramfs_t* fs, const char* mount_point);
bool ramfs_umount(ramfs_t* fs);

ramfs_inode_t* ramfs_create_file(ramfs_t* fs, const char* path, uint32_t permissions);
ramfs_inode_t* ramfs_create_dir(ramfs_t* fs, const char* path, uint32_t permissions);
ramfs_inode_t* ramfs_create_symlink(ramfs_t* fs, const char* path, const char* target);

bool ramfs_delete(ramfs_t* fs, const char* path);
ramfs_inode_t* ramfs_find(ramfs_t* fs, const char* path);

ramfs_file_t* ramfs_open(ramfs_t* fs, const char* path, uint32_t flags);
bool ramfs_close(ramfs_file_t* file);
uint32_t ramfs_read(ramfs_file_t* file, uint8_t* buffer, uint32_t size);
uint32_t ramfs_write(ramfs_file_t* file, const uint8_t* buffer, uint32_t size);
bool ramfs_seek(ramfs_file_t* file, uint32_t offset);

bool ramfs_list_dir(ramfs_t* fs, const char* path, ramfs_inode_t** entries, uint32_t* count);
uint32_t ramfs_get_free_space(ramfs_t* fs);
uint32_t ramfs_get_used_space(ramfs_t* fs);

/* Előre betöltött struktúra */
bool ramfs_load_initrd(ramfs_t* fs, uint8_t* data, uint32_t size);
bool ramfs_create_default_structure(ramfs_t* fs);

#endif
