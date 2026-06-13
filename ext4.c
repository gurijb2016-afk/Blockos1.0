#include "ext4.h"
#include "../kernel/generic.h"
#include <string.h>

extern bool disk_read(uint32_t lba, uint8_t* buf, uint32_t count);
extern bool disk_write(uint32_t lba, const uint8_t* buf, uint32_t count);

/* Szektort olvas */
static bool ext4_read_block(ext4_fs_t* fs, uint32_t block, uint8_t* buf) {
    uint32_t lba = block * (fs->block_size / 512);
    return disk_read(lba, buf, fs->block_size / 512);
}

static bool ext4_write_block(ext4_fs_t* fs, uint32_t block, const uint8_t* buf) {
    uint32_t lba = block * (fs->block_size / 512);
    return disk_write(lba, buf, fs->block_size / 512);
}

/* Inicializálás */
bool ext4_init(ext4_fs_t* fs) {
    if (!fs) return false;
    memset(fs, 0, sizeof(ext4_fs_t));
    return true;
}

/* Mount */
bool ext4_mount(ext4_fs_t* fs, const char* point) {
    if (!fs || !point) return false;

    /* Superblock olvasás - szektor 2 (1024 byte offset) */
    uint8_t buf[1024];
    if (!disk_read(2, buf, 2)) return false;
    memcpy(&fs->sb, buf, sizeof(ext4_superblock_t));

    /* Validálás */
    if (fs->sb.magic != EXT4_MAGIC) return false;

    /* Paraméterek */
    fs->block_size        = 1024 << fs->sb.log_block_size;
    fs->blocks_per_group  = fs->sb.blocks_per_group;
    fs->inodes_per_group  = fs->sb.inodes_per_group;
    fs->group_count       = (fs->sb.block_count_lo + fs->blocks_per_group - 1) / fs->blocks_per_group;
    fs->first_data_block  = fs->sb.first_data_block;

    strncpy(fs->mount_point, point, 255);
    fs->mounted = true;
    return true;
}

bool ext4_umount(ext4_fs_t* fs) {
    if (!fs) return false;
    fs->mounted = false;
    return true;
}

/* Inode olvasás */
bool ext4_get_inode(ext4_fs_t* fs, uint32_t num, ext4_inode_t* inode) {
    if (!fs || !inode || num < 1) return false;

    uint32_t group = (num - 1) / fs->inodes_per_group;
    uint32_t idx   = (num - 1) % fs->inodes_per_group;

    /* Group descriptor olvasás */
    uint8_t gd_buf[EXT4_BLOCK_SIZE];
    uint32_t gd_block = fs->first_data_block + 1;
    if (!ext4_read_block(fs, gd_block, gd_buf)) return false;

    ext4_group_desc_t* gd = (ext4_group_desc_t*)(gd_buf + group * sizeof(ext4_group_desc_t));

    /* Inode tábla blokk */
    uint32_t inode_table_block = gd->inode_table_lo;
    uint32_t inode_block = inode_table_block + (idx * fs->sb.inode_size) / fs->block_size;
    uint32_t inode_offset = (idx * fs->sb.inode_size) % fs->block_size;

    uint8_t inode_buf[EXT4_BLOCK_SIZE];
    if (!ext4_read_block(fs, inode_block, inode_buf)) return false;

    memcpy(inode, inode_buf + inode_offset, sizeof(ext4_inode_t));
    return true;
}

/* Path alapján inode keresés */
static uint32_t ext4_path_to_inode(ext4_fs_t* fs, const char* path) {
    if (!fs || !path) return 0;
    if (strcmp(path, "/") == 0) return EXT4_ROOT_INODE;

    uint32_t current_inode = EXT4_ROOT_INODE;
    char tmp[256];
    strncpy(tmp, path, 255);

    char* token = strtok(tmp + 1, "/");
    while (token) {
        ext4_inode_t inode;
        if (!ext4_get_inode(fs, current_inode, &inode)) return 0;
        if (!(inode.mode & EXT4_S_IFDIR)) return 0;

        /* Extent alapú blokk olvasás */
        ext4_extent_header_t* eh = (ext4_extent_header_t*)inode.block;
        if (eh->magic != 0xF30A) return 0;

        ext4_extent_t* ext = (ext4_extent_t*)(inode.block + sizeof(ext4_extent_header_t));
        uint8_t dir_buf[EXT4_BLOCK_SIZE];
        uint32_t phys_block = ext->start_lo;
        if (!ext4_read_block(fs, phys_block, dir_buf)) return 0;

        /* Bejegyzések keresése */
        bool found = false;
        uint32_t offset = 0;
        while (offset < fs->block_size) {
            ext4_dir_entry_t* de = (ext4_dir_entry_t*)(dir_buf + offset);
            if (de->rec_len == 0) break;
            if (de->inode != 0 && de->name_len == strlen(token) &&
                strncmp(de->name, token, de->name_len) == 0) {
                current_inode = de->inode;
                found = true;
                break;
            }
            offset += de->rec_len;
        }
        if (!found) return 0;
        token = strtok(NULL, "/");
    }
    return current_inode;
}

/* Fájl megnyitás */
ext4_file_t* ext4_open(ext4_fs_t* fs, const char* path, uint32_t flags) {
    if (!fs || !path || !fs->mounted) return NULL;

    uint32_t inum = ext4_path_to_inode(fs, path);

    /* Ha nem létezik és create flag van */
    if (!inum && (flags & 0x01)) {
        inum = ext4_alloc_inode(fs);
        if (!inum) return NULL;
    }
    if (!inum) return NULL;

    ext4_file_t* file = (ext4_file_t*)malloc(sizeof(ext4_file_t));
    if (!file) return NULL;

    file->inode_num = inum;
    file->position  = 0;
    file->flags     = flags;

    if (!ext4_get_inode(fs, inum, &file->inode)) {
        free(file); return NULL;
    }
    return file;
}

bool ext4_close(ext4_file_t* file) {
    if (!file) return false;
    free(file);
    return true;
}

/* Olvasás */
uint32_t ext4_read(ext4_fs_t* fs, ext4_file_t* file, uint8_t* buf, uint32_t size) {
    if (!fs || !file || !buf || !fs->mounted) return 0;

    uint64_t file_size = file->inode.size_lo | ((uint64_t)file->inode.size_hi << 32);
    if (file->position >= file_size) return 0;

    uint32_t available = file_size - file->position;
    if (size > available) size = available;

    /* Extent alapú olvasás */
    ext4_extent_header_t* eh = (ext4_extent_header_t*)file->inode.block;
    if (eh->magic != 0xF30A) return 0;

    ext4_extent_t* extents = (ext4_extent_t*)(file->inode.block + sizeof(ext4_extent_header_t));
    uint32_t bytes_read = 0;

    for (uint16_t i = 0; i < eh->entries && bytes_read < size; i++) {
        uint64_t ext_start = (uint64_t)extents[i].block * fs->block_size;
        uint64_t ext_size  = (uint64_t)extents[i].len  * fs->block_size;

        if (file->position >= ext_start + ext_size) continue;

        uint32_t ext_offset = (file->position > ext_start) ?
            (file->position - ext_start) : 0;
        uint32_t phys_block = extents[i].start_lo + ext_offset / fs->block_size;
        uint32_t block_off  = ext_offset % fs->block_size;

        uint8_t block_buf[EXT4_BLOCK_SIZE];
        while (bytes_read < size && block_off <= fs->block_size) {
            if (!ext4_read_block(fs, phys_block, block_buf)) return bytes_read;
            uint32_t to_copy = fs->block_size - block_off;
            if (to_copy > size - bytes_read) to_copy = size - bytes_read;
            memcpy(buf + bytes_read, block_buf + block_off, to_copy);
            bytes_read     += to_copy;
            file->position += to_copy;
            block_off = 0;
            phys_block++;
        }
    }
    return bytes_read;
}

/* Írás */
uint32_t ext4_write(ext4_fs_t* fs, ext4_file_t* file, const uint8_t* buf, uint32_t size) {
    if (!fs || !file || !buf || !fs->mounted) return 0;
    /* Blokk allokáció + extent frissítés itt lenne */
    file->position += size;
    return size;
}

bool ext4_seek(ext4_file_t* file, uint64_t offset) {
    if (!file) return false;
    uint64_t size = file->inode.size_lo | ((uint64_t)file->inode.size_hi << 32);
    if (offset > size) return false;
    file->position = offset;
    return true;
}

/* Könyvtár listázás */
bool ext4_list_dir(ext4_fs_t* fs, const char* path,
                   ext4_dir_entry_t* entries, uint32_t* count) {
    if (!fs || !path || !entries || !count) return false;

    uint32_t inum = ext4_path_to_inode(fs, path);
    if (!inum) return false;

    ext4_inode_t inode;
    if (!ext4_get_inode(fs, inum, &inode)) return false;
    if (!(inode.mode & EXT4_S_IFDIR)) return false;

    ext4_extent_header_t* eh = (ext4_extent_header_t*)inode.block;
    if (eh->magic != 0xF30A) return false;

    ext4_extent_t* ext = (ext4_extent_t*)(inode.block + sizeof(ext4_extent_header_t));
    uint8_t dir_buf[EXT4_BLOCK_SIZE];
    if (!ext4_read_block(fs, ext->start_lo, dir_buf)) return false;

    uint32_t n = 0, offset = 0;
    while (offset < fs->block_size && n < *count) {
        ext4_dir_entry_t* de = (ext4_dir_entry_t*)(dir_buf + offset);
        if (de->rec_len == 0) break;
        if (de->inode != 0) {
            memcpy(&entries[n], de, sizeof(ext4_dir_entry_t));
            entries[n].name[de->name_len] = 0;
            n++;
        }
        offset += de->rec_len;
    }
    *count = n;
    return true;
}

/* Könyvtár létrehozás */
bool ext4_mkdir(ext4_fs_t* fs, const char* path, uint16_t mode) {
    if (!fs || !path || !fs->mounted) return false;
    uint32_t inum = ext4_alloc_inode(fs);
    if (!inum) return false;
    /* Inode inicializálás, parent dir frissítés itt lenne */
    (void)mode;
    return true;
}

bool ext4_rmdir(ext4_fs_t* fs, const char* path) {
    if (!fs || !path) return false;
    uint32_t inum = ext4_path_to_inode(fs, path);
    if (!inum) return false;
    return ext4_free_inode(fs, inum);
}

bool ext4_unlink(ext4_fs_t* fs, const char* path) {
    if (!fs || !path) return false;
    uint32_t inum = ext4_path_to_inode(fs, path);
    if (!inum) return false;
    ext4_inode_t inode;
    if (!ext4_get_inode(fs, inum, &inode)) return false;
    inode.links_count--;
    if (inode.links_count == 0) ext4_free_inode(fs, inum);
    return true;
}

bool ext4_rename(ext4_fs_t* fs, const char* old, const char* newp) {
    (void)fs; (void)old; (void)newp;
    return true;
}

bool ext4_symlink(ext4_fs_t* fs, const char* target, const char* link) {
    (void)fs; (void)target; (void)link;
    return true;
}

/* Inode allokálás */
uint32_t ext4_alloc_inode(ext4_fs_t* fs) {
    if (!fs) return 0;
    /* Bitmap keresés és allokálás itt lenne */
    return 0;
}

uint32_t ext4_alloc_block(ext4_fs_t* fs) {
    if (!fs) return 0;
    return 0;
}

bool ext4_free_inode(ext4_fs_t* fs, uint32_t num) {
    if (!fs || num < 1) return false;
    fs->sb.free_inode_count++;
    return true;
}

bool ext4_free_block(ext4_fs_t* fs, uint32_t block) {
    if (!fs) return false;
    fs->sb.free_block_count_lo++;
    return true;
}

/* Statisztikák */
uint64_t ext4_get_free_space(ext4_fs_t* fs) {
    if (!fs || !fs->mounted) return 0;
    return (uint64_t)fs->sb.free_block_count_lo * fs->block_size;
}

uint64_t ext4_get_total_space(ext4_fs_t* fs) {
    if (!fs || !fs->mounted) return 0;
    return (uint64_t)fs->sb.block_count_lo * fs->block_size;
}

char* ext4_get_label(ext4_fs_t* fs) {
    if (!fs || !fs->mounted) return NULL;
    return fs->sb.volume_name;
}
