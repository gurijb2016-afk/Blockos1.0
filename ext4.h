#ifndef EXT4_H
#define EXT4_H
#include "../kernel/types.h"

/* EXT4 Magic */
#define EXT4_MAGIC          0xEF53
#define EXT4_BLOCK_SIZE     4096
#define EXT4_INODE_SIZE     256
#define EXT4_MAX_FILENAME   255
#define EXT4_ROOT_INODE     2

/* Inode típusok */
#define EXT4_S_IFREG        0x8000  /* Fájl */
#define EXT4_S_IFDIR        0x4000  /* Könyvtár */
#define EXT4_S_IFLNK        0xA000  /* Symlink */

/* Feature flags */
#define EXT4_FEAT_EXTENTS   0x0040
#define EXT4_FEAT_64BIT     0x0080
#define EXT4_FEAT_FLEX_BG   0x0200

/* Superblock */
typedef struct {
    uint32_t inode_count;
    uint32_t block_count_lo;
    uint32_t r_block_count_lo;
    uint32_t free_block_count_lo;
    uint32_t free_inode_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_cluster_size;
    uint32_t blocks_per_group;
    uint32_t clusters_per_group;
    uint32_t inodes_per_group;
    uint32_t mount_time;
    uint32_t write_time;
    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t  uuid[16];
    char     volume_name[16];
    char     last_mounted[64];
    uint32_t algorithm_usage_bitmap;
    uint8_t  prealloc_blocks;
    uint8_t  prealloc_dir_blocks;
    uint16_t reserved_gdt_blocks;
    uint8_t  journal_uuid[16];
    uint32_t journal_inum;
    uint32_t journal_dev;
    uint32_t last_orphan;
    uint32_t hash_seed[4];
    uint8_t  def_hash_version;
    uint8_t  jnl_backup_type;
    uint16_t desc_size;
    uint32_t default_mount_opts;
    uint32_t first_meta_bg;
    uint32_t mkfs_time;
    uint32_t jnl_blocks[17];
} __attribute__((packed)) ext4_superblock_t;

/* Block Group Descriptor */
typedef struct {
    uint32_t block_bitmap_lo;
    uint32_t inode_bitmap_lo;
    uint32_t inode_table_lo;
    uint16_t free_blocks_count_lo;
    uint16_t free_inodes_count_lo;
    uint16_t used_dirs_count_lo;
    uint16_t flags;
    uint32_t exclude_bitmap_lo;
    uint16_t block_bitmap_csum_lo;
    uint16_t inode_bitmap_csum_lo;
    uint16_t itable_unused_lo;
    uint16_t checksum;
} __attribute__((packed)) ext4_group_desc_t;

/* Extent fejléc */
typedef struct {
    uint16_t magic;
    uint16_t entries;
    uint16_t max;
    uint16_t depth;
    uint32_t generation;
} __attribute__((packed)) ext4_extent_header_t;

/* Extent */
typedef struct {
    uint32_t block;
    uint16_t len;
    uint16_t start_hi;
    uint32_t start_lo;
} __attribute__((packed)) ext4_extent_t;

/* Extent index */
typedef struct {
    uint32_t block;
    uint32_t leaf_lo;
    uint16_t leaf_hi;
    uint16_t unused;
} __attribute__((packed)) ext4_extent_idx_t;

/* Inode */
typedef struct {
    uint16_t mode;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks_lo;
    uint32_t flags;
    uint32_t osd1;
    uint8_t  block[60];  /* Extent tree vagy blokk mutatók */
    uint32_t generation;
    uint32_t file_acl_lo;
    uint32_t size_hi;
    uint32_t obso_faddr;
    uint8_t  osd2[12];
    uint16_t extra_isize;
    uint16_t checksum_hi;
    uint32_t ctime_extra;
    uint32_t mtime_extra;
    uint32_t atime_extra;
    uint32_t crtime;
    uint32_t crtime_extra;
    uint32_t version_hi;
    uint32_t projid;
} __attribute__((packed)) ext4_inode_t;

/* Directory bejegyzés */
typedef struct {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[EXT4_MAX_FILENAME];
} __attribute__((packed)) ext4_dir_entry_t;

/* Fájl leíró */
typedef struct {
    uint32_t inode_num;
    ext4_inode_t inode;
    uint64_t position;
    uint32_t flags;
} ext4_file_t;

/* EXT4 fájlrendszer */
typedef struct {
    bool              mounted;
    ext4_superblock_t sb;
    uint32_t          block_size;
    uint32_t          blocks_per_group;
    uint32_t          inodes_per_group;
    uint32_t          group_count;
    uint32_t          first_data_block;
    char              mount_point[256];
} ext4_fs_t;

/* Függvények */
bool ext4_init(ext4_fs_t* fs);
bool ext4_mount(ext4_fs_t* fs, const char* point);
bool ext4_umount(ext4_fs_t* fs);

ext4_file_t* ext4_open(ext4_fs_t* fs, const char* path, uint32_t flags);
bool ext4_close(ext4_file_t* file);
uint32_t ext4_read(ext4_fs_t* fs, ext4_file_t* file, uint8_t* buf, uint32_t size);
uint32_t ext4_write(ext4_fs_t* fs, ext4_file_t* file, const uint8_t* buf, uint32_t size);
bool ext4_seek(ext4_file_t* file, uint64_t offset);

bool ext4_mkdir(ext4_fs_t* fs, const char* path, uint16_t mode);
bool ext4_rmdir(ext4_fs_t* fs, const char* path);
bool ext4_unlink(ext4_fs_t* fs, const char* path);
bool ext4_rename(ext4_fs_t* fs, const char* old, const char* new);
bool ext4_symlink(ext4_fs_t* fs, const char* target, const char* link);
bool ext4_list_dir(ext4_fs_t* fs, const char* path, ext4_dir_entry_t* entries, uint32_t* count);

bool ext4_get_inode(ext4_fs_t* fs, uint32_t num, ext4_inode_t* inode);
uint32_t ext4_alloc_inode(ext4_fs_t* fs);
uint32_t ext4_alloc_block(ext4_fs_t* fs);
bool ext4_free_inode(ext4_fs_t* fs, uint32_t num);
bool ext4_free_block(ext4_fs_t* fs, uint32_t block);

uint64_t ext4_get_free_space(ext4_fs_t* fs);
uint64_t ext4_get_total_space(ext4_fs_t* fs);
char*    ext4_get_label(ext4_fs_t* fs);

#endif
