#ifndef VFS_H
#define VFS_H
#include "../kernel/types.h"

#define VFS_MAX_MOUNTS   16
#define VFS_MAX_FDS      256
#define VFS_MAX_PATH     1024
#define VFS_MAX_NAME     256

/* Fájlrendszer típusok */
#define VFS_FS_RAMFS  0
#define VFS_FS_FAT32  1
#define VFS_FS_EXT4   2
#define VFS_FS_PROCFS 3
#define VFS_FS_DEVFS  4

/* Fájl típusok */
#define VFS_TYPE_FILE   0
#define VFS_TYPE_DIR    1
#define VFS_TYPE_LINK   2
#define VFS_TYPE_DEVICE 3
#define VFS_TYPE_PIPE   4

/* Open flagek */
#define VFS_O_RDONLY  0x00
#define VFS_O_WRONLY  0x01
#define VFS_O_RDWR    0x02
#define VFS_O_CREAT   0x04
#define VFS_O_TRUNC   0x08
#define VFS_O_APPEND  0x10

/* VFS node */
typedef struct vfs_node {
    char     name[VFS_MAX_NAME];
    uint8_t  type;
    uint32_t inode;
    uint32_t size;
    uint32_t uid, gid;
    uint32_t permissions;
    uint64_t atime, mtime, ctime;
    void*    fs_data;
    uint8_t  fs_type;

    /* Műveletek */
    uint32_t (*read) (struct vfs_node*, uint8_t*, uint32_t, uint32_t);
    uint32_t (*write)(struct vfs_node*, const uint8_t*, uint32_t, uint32_t);
    bool     (*open) (struct vfs_node*, uint32_t);
    bool     (*close)(struct vfs_node*);
    bool     (*mkdir)(struct vfs_node*, const char*, uint32_t);
    bool     (*unlink)(struct vfs_node*, const char*);
    struct vfs_node* (*finddir)(struct vfs_node*, const char*);
    struct vfs_node* (*readdir)(struct vfs_node*, uint32_t);
} vfs_node_t;

/* Mount pont */
typedef struct {
    char       path[VFS_MAX_PATH];
    uint8_t    fs_type;
    void*      fs_data;
    vfs_node_t* root;
    bool       active;
    bool       read_only;
} vfs_mount_t;

/* Fájl leíró */
typedef struct {
    vfs_node_t* node;
    uint32_t    position;
    uint32_t    flags;
    bool        active;
} vfs_fd_t;

/* VFS állapot */
typedef struct {
    vfs_mount_t mounts[VFS_MAX_MOUNTS];
    uint32_t    mount_count;
    vfs_fd_t    fds[VFS_MAX_FDS];
    vfs_node_t* root;
} vfs_t;

/* Függvények */
void vfs_init(void);
bool vfs_mount(const char* path, uint8_t fs_type, void* fs_data);
bool vfs_umount(const char* path);

int      vfs_open(const char* path, uint32_t flags);
bool     vfs_close(int fd);
uint32_t vfs_read(int fd, uint8_t* buf, uint32_t size);
uint32_t vfs_write(int fd, const uint8_t* buf, uint32_t size);
bool     vfs_seek(int fd, uint32_t offset);
uint32_t vfs_tell(int fd);

bool     vfs_mkdir(const char* path, uint32_t mode);
bool     vfs_rmdir(const char* path);
bool     vfs_unlink(const char* path);
bool     vfs_rename(const char* old, const char* newp);
bool     vfs_stat(const char* path, vfs_node_t* out);
bool     vfs_exists(const char* path);

vfs_node_t* vfs_resolve(const char* path);
vfs_mount_t* vfs_find_mount(const char* path);

bool vfs_mount_fat32(int drive);
bool vfs_mount_ext4(const char* dev, const char* point);
bool vfs_mount_ramfs(const char* point);
bool vfs_mount_procfs(const char* point);

#endif
