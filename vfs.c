#include "vfs.h"
#include "fat32.h"
#include "ext4.h"
#include "ramfs.h"
#include "procfs.h"
#include "../kernel/generic.h"

static vfs_t g_vfs;

void vfs_init(void) {
    memset(&g_vfs, 0, sizeof(vfs_t));
}

/* Mount pont keresés */
vfs_mount_t* vfs_find_mount(const char* path) {
    vfs_mount_t* best = NULL;
    int best_len = -1;
    for (uint32_t i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (!g_vfs.mounts[i].active) continue;
        int len = strlen(g_vfs.mounts[i].path);
        if (strncmp(path, g_vfs.mounts[i].path, len) == 0 && len > best_len) {
            best = &g_vfs.mounts[i];
            best_len = len;
        }
    }
    return best;
}

/* Mount */
bool vfs_mount(const char* path, uint8_t fs_type, void* fs_data) {
    for (uint32_t i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (!g_vfs.mounts[i].active) {
            strncpy(g_vfs.mounts[i].path, path, VFS_MAX_PATH-1);
            g_vfs.mounts[i].fs_type  = fs_type;
            g_vfs.mounts[i].fs_data  = fs_data;
            g_vfs.mounts[i].active   = true;
            g_vfs.mount_count++;
            return true;
        }
    }
    return false;
}

bool vfs_umount(const char* path) {
    for (uint32_t i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (g_vfs.mounts[i].active && strcmp(g_vfs.mounts[i].path, path) == 0) {
            g_vfs.mounts[i].active = false;
            g_vfs.mount_count--;
            return true;
        }
    }
    return false;
}

/* FD allokálás */
static int vfs_alloc_fd(void) {
    for (int i = 0; i < VFS_MAX_FDS; i++)
        if (!g_vfs.fds[i].active) return i;
    return -1;
}

/* Fájl megnyitás */
int vfs_open(const char* path, uint32_t flags) {
    vfs_mount_t* mount = vfs_find_mount(path);
    if (!mount) return -1;

    int fd = vfs_alloc_fd();
    if (fd < 0) return -1;

    /* Relatív path a mount ponton belül */
    const char* rel_path = path + strlen(mount->path);
    if (!rel_path[0]) rel_path = "/";

    g_vfs.fds[fd].position = 0;
    g_vfs.fds[fd].flags    = flags;
    g_vfs.fds[fd].active   = true;

    /* FS specifikus open */
    switch (mount->fs_type) {
        case VFS_FS_EXT4: {
            ext4_file_t* f = ext4_open((ext4_fs_t*)mount->fs_data, rel_path, flags);
            if (!f) { g_vfs.fds[fd].active = false; return -1; }
            break;
        }
        case VFS_FS_FAT32: {
            FAT32File* f = (FAT32File*)malloc(sizeof(FAT32File));
            if (!fat32_open((FAT32FileSystem*)mount->fs_data, rel_path, f)) {
                free(f); g_vfs.fds[fd].active = false; return -1;
            }
            break;
        }
        case VFS_FS_RAMFS: {
            ramfs_file_t* f = ramfs_open((ramfs_t*)mount->fs_data, rel_path, flags);
            if (!f) { g_vfs.fds[fd].active = false; return -1; }
            break;
        }
        case VFS_FS_PROCFS: {
            procfs_file_t* f = procfs_open((procfs_t*)mount->fs_data, rel_path);
            if (!f) { g_vfs.fds[fd].active = false; return -1; }
            break;
        }
    }
    return fd;
}

bool vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_vfs.fds[fd].active) return false;
    g_vfs.fds[fd].active = false;
    return true;
}

uint32_t vfs_read(int fd, uint8_t* buf, uint32_t size) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_vfs.fds[fd].active) return 0;
    /* FS specifikus read */
    return 0;
}

uint32_t vfs_write(int fd, const uint8_t* buf, uint32_t size) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !g_vfs.fds[fd].active) return 0;
    /* FS specifikus write */
    return 0;
}

bool vfs_seek(int fd, uint32_t offset) {
    if (fd < 0 || fd >= VFS_MAX_FDS) return false;
    g_vfs.fds[fd].position = offset;
    return true;
}

uint32_t vfs_tell(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS) return 0;
    return g_vfs.fds[fd].position;
}

bool vfs_mkdir(const char* path, uint32_t mode) {
    vfs_mount_t* mount = vfs_find_mount(path);
    if (!mount) return false;
    const char* rel = path + strlen(mount->path);
    switch (mount->fs_type) {
        case VFS_FS_EXT4:  return ext4_mkdir((ext4_fs_t*)mount->fs_data, rel, mode);
        case VFS_FS_RAMFS: return ramfs_create_dir((ramfs_t*)mount->fs_data, rel, mode) != NULL;
    }
    return false;
}

bool vfs_unlink(const char* path) {
    vfs_mount_t* mount = vfs_find_mount(path);
    if (!mount) return false;
    const char* rel = path + strlen(mount->path);
    switch (mount->fs_type) {
        case VFS_FS_EXT4:  return ext4_unlink((ext4_fs_t*)mount->fs_data, rel);
        case VFS_FS_RAMFS: return ramfs_delete((ramfs_t*)mount->fs_data, rel);
    }
    return false;
}

bool vfs_exists(const char* path) {
    vfs_mount_t* mount = vfs_find_mount(path);
    if (!mount) return false;
    const char* rel = path + strlen(mount->path);
    switch (mount->fs_type) {
        case VFS_FS_EXT4:  return ext4_path_to_inode((ext4_fs_t*)mount->fs_data, rel) != 0;
        case VFS_FS_RAMFS: return ramfs_find((ramfs_t*)mount->fs_data, rel) != NULL;
    }
    return false;
}

bool vfs_rename(const char* old, const char* newp) {
    vfs_mount_t* m = vfs_find_mount(old);
    if (!m) return false;
    switch (m->fs_type) {
        case VFS_FS_EXT4: return ext4_rename((ext4_fs_t*)m->fs_data,
            old + strlen(m->path), newp + strlen(m->path));
    }
    return false;
}

/* Helper mount függvények */
bool vfs_mount_fat32(int drive) {
    FAT32FileSystem* fs = (FAT32FileSystem*)malloc(sizeof(FAT32FileSystem));
    if (!fs) return false;
    fat32_init(fs);
    if (!fat32_mount(fs)) { free(fs); return false; }
    return vfs_mount("/mnt", VFS_FS_FAT32, fs);
}

bool vfs_mount_ext4(const char* dev, const char* point) {
    ext4_fs_t* fs = (ext4_fs_t*)malloc(sizeof(ext4_fs_t));
    if (!fs) return false;
    ext4_init(fs);
    if (!ext4_mount(fs, point)) { free(fs); return false; }
    return vfs_mount(point, VFS_FS_EXT4, fs);
}

bool vfs_mount_ramfs(const char* point) {
    ramfs_t* fs = (ramfs_t*)malloc(sizeof(ramfs_t));
    if (!fs) return false;
    ramfs_init(fs);
    ramfs_mount(fs, point);
    ramfs_create_default_structure(fs);
    return vfs_mount(point, VFS_FS_RAMFS, fs);
}

bool vfs_mount_procfs(const char* point) {
    procfs_t* fs = (procfs_t*)malloc(sizeof(procfs_t));
    if (!fs) return false;
    procfs_init(fs);
    procfs_mount(fs, point);
    return vfs_mount(point, VFS_FS_PROCFS, fs);
}

bool vfs_stat(const char* path, vfs_node_t* out) {
    if (!path || !out) return false;
    vfs_mount_t* mount = vfs_find_mount(path);
    if (!mount) return false;
    memset(out, 0, sizeof(vfs_node_t));
    strncpy(out->name, path, VFS_MAX_NAME-1);
    out->fs_type = mount->fs_type;
    return true;
}

vfs_node_t* vfs_resolve(const char* path) {
    (void)path;
    return NULL;
}
