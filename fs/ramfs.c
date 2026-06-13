#include "ramfs.h"
#include "../kernel/types.h"
#include <string.h>

/* Globális RAMFS instance */
static ramfs_t* g_ramfs = NULL;
static uint32_t next_inode_id = 1;

/* Helper: path szétbontása */
static void split_path(const char* path, char* parent, char* name) {
    int len = strlen(path);
    int last_slash = -1;

    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '/') { last_slash = i; break; }
    }

    if (last_slash <= 0) {
        strcpy(parent, "/");
        strcpy(name, path + (last_slash + 1));
    } else {
        strncpy(parent, path, last_slash);
        parent[last_slash] = 0;
        strcpy(name, path + last_slash + 1);
    }
}

/* Helper: új inode létrehozása */
static ramfs_inode_t* alloc_inode(const char* name, uint8_t type, uint32_t permissions) {
    ramfs_inode_t* node = (ramfs_inode_t*)malloc(sizeof(ramfs_inode_t));
    if (!node) return NULL;

    memset(node, 0, sizeof(ramfs_inode_t));
    node->id = next_inode_id++;
    strncpy(node->name, name, RAMFS_MAX_NAME - 1);
    node->type = type;
    node->permissions = permissions;
    node->size = 0;
    node->data = NULL;
    node->children = NULL;
    node->next = NULL;
    node->parent = NULL;

    return node;
}

/* Inode keresése path alapján */
static ramfs_inode_t* find_inode(ramfs_t* fs, const char* path) {
    if (!fs || !path) return NULL;
    if (strcmp(path, "/") == 0) return fs->root;

    ramfs_inode_t* current = fs->root;
    char tmp[RAMFS_MAX_PATH];
    strncpy(tmp, path, RAMFS_MAX_PATH - 1);

    char* token = strtok(tmp, "/");
    while (token && current) {
        ramfs_inode_t* child = current->children;
        bool found = false;
        while (child) {
            if (strcmp(child->name, token) == 0) {
                current = child;
                found = true;
                break;
            }
            child = child->next;
        }
        if (!found) return NULL;
        token = strtok(NULL, "/");
    }

    return current;
}

/* Child hozzáadása szülőhöz */
static void add_child(ramfs_inode_t* parent, ramfs_inode_t* child) {
    if (!parent || !child) return;
    child->parent = parent;
    child->next = parent->children;
    parent->children = child;
}

/* RAMFS inicializálás */
bool ramfs_init(ramfs_t* fs) {
    if (!fs) return false;

    memset(fs, 0, sizeof(ramfs_t));

    /* Root inode létrehozása */
    fs->root = alloc_inode("/", RAMFS_TYPE_DIR, 0755);
    if (!fs->root) return false;

    fs->mounted = false;
    fs->total_size = RAMFS_MAX_BLOCKS * RAMFS_BLOCK_SIZE;
    fs->used_size = 0;
    fs->file_count = 0;
    fs->dir_count = 1;

    g_ramfs = fs;
    return true;
}

/* Mount */
bool ramfs_mount(ramfs_t* fs, const char* mount_point) {
    if (!fs || !mount_point) return false;
    strncpy(fs->mount_point, mount_point, RAMFS_MAX_PATH - 1);
    fs->mounted = true;
    return true;
}

/* Umount */
bool ramfs_umount(ramfs_t* fs) {
    if (!fs) return false;
    fs->mounted = false;
    return true;
}

/* Fájl létrehozása */
ramfs_inode_t* ramfs_create_file(ramfs_t* fs, const char* path, uint32_t permissions) {
    if (!fs || !path) return NULL;

    char parent_path[RAMFS_MAX_PATH];
    char name[RAMFS_MAX_NAME];
    split_path(path, parent_path, name);

    ramfs_inode_t* parent = find_inode(fs, parent_path);
    if (!parent || parent->type != RAMFS_TYPE_DIR) return NULL;

    ramfs_inode_t* file = alloc_inode(name, RAMFS_TYPE_FILE, permissions);
    if (!file) return NULL;

    add_child(parent, file);
    fs->file_count++;

    return file;
}

/* Könyvtár létrehozása */
ramfs_inode_t* ramfs_create_dir(ramfs_t* fs, const char* path, uint32_t permissions) {
    if (!fs || !path) return NULL;

    char parent_path[RAMFS_MAX_PATH];
    char name[RAMFS_MAX_NAME];
    split_path(path, parent_path, name);

    ramfs_inode_t* parent = find_inode(fs, parent_path);
    if (!parent || parent->type != RAMFS_TYPE_DIR) return NULL;

    ramfs_inode_t* dir = alloc_inode(name, RAMFS_TYPE_DIR, permissions);
    if (!dir) return NULL;

    add_child(parent, dir);
    fs->dir_count++;

    return dir;
}

/* Symlink létrehozása */
ramfs_inode_t* ramfs_create_symlink(ramfs_t* fs, const char* path, const char* target) {
    if (!fs || !path || !target) return NULL;

    char parent_path[RAMFS_MAX_PATH];
    char name[RAMFS_MAX_NAME];
    split_path(path, parent_path, name);

    ramfs_inode_t* parent = find_inode(fs, parent_path);
    if (!parent) return NULL;

    ramfs_inode_t* link = alloc_inode(name, RAMFS_TYPE_SYMLINK, 0777);
    if (!link) return NULL;

    strncpy(link->symlink_target, target, RAMFS_MAX_PATH - 1);
    add_child(parent, link);

    return link;
}

/* Inode keresése */
ramfs_inode_t* ramfs_find(ramfs_t* fs, const char* path) {
    return find_inode(fs, path);
}

/* Fájl megnyitása */
ramfs_file_t* ramfs_open(ramfs_t* fs, const char* path, uint32_t flags) {
    if (!fs || !path) return NULL;

    ramfs_inode_t* inode = find_inode(fs, path);
    if (!inode && (flags & 0x01)) {
        inode = ramfs_create_file(fs, path, 0644);
    }
    if (!inode) return NULL;

    ramfs_file_t* file = (ramfs_file_t*)malloc(sizeof(ramfs_file_t));
    if (!file) return NULL;

    file->inode = inode;
    file->position = 0;
    file->flags = flags;

    return file;
}

/* Fájl bezárása */
bool ramfs_close(ramfs_file_t* file) {
    if (!file) return false;
    free(file);
    return true;
}

/* Olvasás */
uint32_t ramfs_read(ramfs_file_t* file, uint8_t* buffer, uint32_t size) {
    if (!file || !buffer || !file->inode) return 0;

    uint32_t available = file->inode->size - file->position;
    if (available == 0) return 0;
    if (size > available) size = available;

    memcpy(buffer, file->inode->data + file->position, size);
    file->position += size;

    return size;
}

/* Írás */
uint32_t ramfs_write(ramfs_file_t* file, const uint8_t* buffer, uint32_t size) {
    if (!file || !buffer || !file->inode) return 0;

    uint32_t new_size = file->position + size;

    /* Realloc ha szükséges */
    if (new_size > file->inode->data_size) {
        uint8_t* new_data = (uint8_t*)realloc(file->inode->data, new_size);
        if (!new_data) return 0;
        file->inode->data = new_data;
        file->inode->data_size = new_size;
        g_ramfs->used_size += size;
    }

    memcpy(file->inode->data + file->position, buffer, size);
    file->position += size;

    if (file->position > file->inode->size) {
        file->inode->size = file->position;
    }

    return size;
}

/* Seek */
bool ramfs_seek(ramfs_file_t* file, uint32_t offset) {
    if (!file || offset > file->inode->size) return false;
    file->position = offset;
    return true;
}

/* Könyvtár listázása */
bool ramfs_list_dir(ramfs_t* fs, const char* path,
                    ramfs_inode_t** entries, uint32_t* count) {
    if (!fs || !path || !entries || !count) return false;

    ramfs_inode_t* dir = find_inode(fs, path);
    if (!dir || dir->type != RAMFS_TYPE_DIR) return false;

    uint32_t n = 0;
    ramfs_inode_t* child = dir->children;
    while (child && n < *count) {
        entries[n++] = child;
        child = child->next;
    }
    *count = n;

    return true;
}

/* Törlés */
bool ramfs_delete(ramfs_t* fs, const char* path) {
    if (!fs || !path) return false;

    ramfs_inode_t* node = find_inode(fs, path);
    if (!node || !node->parent) return false;

    /* Parent children listából kivesszük */
    ramfs_inode_t* parent = node->parent;
    if (parent->children == node) {
        parent->children = node->next;
    } else {
        ramfs_inode_t* prev = parent->children;
        while (prev && prev->next != node) prev = prev->next;
        if (prev) prev->next = node->next;
    }

    /* Adat felszabadítás */
    if (node->data) {
        g_ramfs->used_size -= node->data_size;
        free(node->data);
    }

    if (node->type == RAMFS_TYPE_FILE) fs->file_count--;
    else if (node->type == RAMFS_TYPE_DIR) fs->dir_count--;

    free(node);
    return true;
}

/* Szabad hely */
uint32_t ramfs_get_free_space(ramfs_t* fs) {
    if (!fs) return 0;
    return fs->total_size - fs->used_size;
}

/* Foglalt hely */
uint32_t ramfs_get_used_space(ramfs_t* fs) {
    if (!fs) return 0;
    return fs->used_size;
}

/* Alapértelmezett BlockOS fájlrendszer struktúra */
bool ramfs_create_default_structure(ramfs_t* fs) {
    if (!fs) return false;

    /* Alap könyvtárak */
    ramfs_create_dir(fs, "/bin",     0755);
    ramfs_create_dir(fs, "/boot",    0755);
    ramfs_create_dir(fs, "/dev",     0755);
    ramfs_create_dir(fs, "/etc",     0755);
    ramfs_create_dir(fs, "/home",    0755);
    ramfs_create_dir(fs, "/lib",     0755);
    ramfs_create_dir(fs, "/mnt",     0755);
    ramfs_create_dir(fs, "/proc",    0555);
    ramfs_create_dir(fs, "/root",    0700);
    ramfs_create_dir(fs, "/sys",     0555);
    ramfs_create_dir(fs, "/tmp",     0777);
    ramfs_create_dir(fs, "/usr",     0755);
    ramfs_create_dir(fs, "/usr/bin", 0755);
    ramfs_create_dir(fs, "/usr/lib", 0755);
    ramfs_create_dir(fs, "/var",     0755);
    ramfs_create_dir(fs, "/var/log", 0755);

    /* BlockOS specifikus */
    ramfs_create_dir(fs, "/blockos",          0755);
    ramfs_create_dir(fs, "/blockos/themes",   0755);
    ramfs_create_dir(fs, "/blockos/wallpaper", 0755);
    ramfs_create_dir(fs, "/blockos/apps",     0755);

    /* /etc fájlok */
    ramfs_inode_t* hostname = ramfs_create_file(fs, "/etc/hostname", 0644);
    if (hostname) {
        const char* name = "blockos\n";
        hostname->data = (uint8_t*)strdup(name);
        hostname->size = strlen(name);
        hostname->data_size = hostname->size;
    }

    ramfs_inode_t* os_release = ramfs_create_file(fs, "/etc/os-release", 0644);
    if (os_release) {
        const char* info =
            "NAME=\"BlockOS\"\n"
            "VERSION=\"1.0\"\n"
            "ID=blockos\n"
            "PRETTY_NAME=\"BlockOS 1.0\"\n"
            "HOME_URL=\"https://sourceforge.net/projects/blockos\"\n";
        os_release->data = (uint8_t*)strdup(info);
        os_release->size = strlen(info);
        os_release->data_size = os_release->size;
    }

    /* Symlinkok */
    ramfs_create_symlink(fs, "/bin/sh",   "/bin/blocksh");
    ramfs_create_symlink(fs, "/usr/bin/cc", "/usr/bin/gcc");

    return true;
}

/* Initrd betöltése RAM-ba */
bool ramfs_load_initrd(ramfs_t* fs, uint8_t* data, uint32_t size) {
    if (!fs || !data || size == 0) return false;

    /* Alap struktúra létrehozása */
    ramfs_create_default_structure(fs);

    /* Initrd adatok betöltése /boot-ba */
    ramfs_inode_t* initrd = ramfs_create_file(fs, "/boot/initrd", 0644);
    if (!initrd) return false;

    initrd->data = (uint8_t*)malloc(size);
    if (!initrd->data) return false;

    memcpy(initrd->data, data, size);
    initrd->size = size;
    initrd->data_size = size;
    fs->used_size += size;

    return true;
}
