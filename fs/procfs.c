#include "procfs.h"
#include "../kernel/process.h"
#include "../kernel/types.h"
#include <string.h>

/* Globális procfs */
static procfs_t* g_procfs = NULL;

/* ============================================
   READ CALLBACK FÜGGVÉNYEK
   ============================================ */

/* /proc/version */
static char* read_version(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "BlockOS 1.0 (blockos@localhost) "
        "(gcc 12.0) #1 SMP %s\n",
        __DATE__);
    return buf;
}

/* /proc/uptime */
static char* read_uptime(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    extern uint64_t kernel_get_ticks(void);
    uint64_t ticks = kernel_get_ticks();
    uint64_t seconds = ticks / 100;
    uint64_t idle = seconds / 2;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "%llu.%02llu %llu.%02llu\n",
        seconds, ticks % 100, idle, (ticks/2) % 100);
    return buf;
}

/* /proc/cpuinfo */
static char* read_cpuinfo(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "processor\t: 0\n"
        "vendor_id\t: BlockOS CPU\n"
        "cpu family\t: 6\n"
        "model name\t: BlockOS Virtual CPU\n"
        "stepping\t: 1\n"
        "cpu MHz\t\t: 1000.000\n"
        "cache size\t: 256 KB\n"
        "flags\t\t: fpu vme de pse tsc msr pae mce\n"
        "bogomips\t: 2000.00\n"
        "clflush size\t: 64\n"
        "cache_alignment\t: 64\n"
        "address sizes\t: 32 bits physical, 32 bits virtual\n");
    return buf;
}

/* /proc/meminfo */
static char* read_meminfo(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    extern uint32_t mem_get_total(void);
    extern uint32_t mem_get_free(void);
    uint32_t total = mem_get_total();
    uint32_t free_mem = mem_get_free();
    uint32_t used = total - free_mem;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "MemTotal:\t%8u kB\n"
        "MemFree:\t%8u kB\n"
        "MemAvailable:\t%8u kB\n"
        "MemUsed:\t%8u kB\n"
        "Buffers:\t%8u kB\n"
        "Cached:\t\t%8u kB\n"
        "SwapTotal:\t%8u kB\n"
        "SwapFree:\t%8u kB\n",
        total / 1024,
        free_mem / 1024,
        free_mem / 1024,
        used / 1024,
        0, 0, 0, 0);
    return buf;
}

/* /proc/stat */
static char* read_stat(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    extern uint64_t kernel_get_ticks(void);
    uint64_t ticks = kernel_get_ticks();
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "cpu  %llu %llu %llu %llu 0 0 0 0 0 0\n"
        "cpu0 %llu %llu %llu %llu 0 0 0 0 0 0\n"
        "intr 0\n"
        "ctxt %llu\n"
        "btime 0\n"
        "processes %u\n"
        "procs_running 1\n"
        "procs_blocked 0\n",
        ticks/4, ticks/8, ticks/8, ticks/2,
        ticks/4, ticks/8, ticks/8, ticks/2,
        ticks, process_count);
    return buf;
}

/* /proc/mounts */
static char* read_mounts(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "ramfs / ramfs rw,relatime 0 0\n"
        "procfs /proc procfs rw,relatime 0 0\n"
        "devfs /dev devfs rw,relatime 0 0\n"
        "fat32 /mnt/disk fat32 rw,relatime 0 0\n");
    return buf;
}

/* /proc/filesystems */
static char* read_filesystems(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "nodev\tramfs\n"
        "nodev\tprocfs\n"
        "nodev\tdevfs\n"
        "\tfat32\n");
    return buf;
}

/* /proc/interrupts */
static char* read_interrupts(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "           CPU0\n"
        "  0:    1000000   Timer\n"
        "  1:      50000   PS/2 Keyboard\n"
        " 12:      25000   PS/2 Mouse\n"
        " 14:       1000   ATA Primary\n"
        " 15:        500   ATA Secondary\n");
    return buf;
}

/* /proc/loadavg */
static char* read_loadavg(uint32_t pid) {
    char* buf = (char*)malloc(256);
    if (!buf) return NULL;
    snprintf(buf, 256, "0.10 0.08 0.05 1/%u %u\n",
        process_count, process_count);
    return buf;
}

/* /proc/cmdline */
static char* read_cmdline(uint32_t pid) {
    char* buf = (char*)malloc(256);
    if (!buf) return NULL;
    snprintf(buf, 256, "blockos root=/dev/sda1 quiet splash\n");
    return buf;
}

/* /proc/PID/status */
static char* read_pid_status(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;

    process_t* proc = process_get_by_pid(pid);
    if (!proc) {
        snprintf(buf, PROCFS_BUFFER_SIZE, "Process not found\n");
        return buf;
    }

    const char* state_str = "unknown";
    switch(proc->state) {
        case PROCESS_STATE_READY:   state_str = "R (running)"; break;
        case PROCESS_STATE_BLOCKED: state_str = "S (sleeping)"; break;
        case PROCESS_STATE_ZOMBIE:  state_str = "Z (zombie)"; break;
        case PROCESS_STATE_DEAD:    state_str = "X (dead)"; break;
        case PROCESS_STATE_NEW:     state_str = "N (new)"; break;
    }

    snprintf(buf, PROCFS_BUFFER_SIZE,
        "Name:\t%s\n"
        "State:\t%s\n"
        "Pid:\t%u\n"
        "PPid:\t%u\n"
        "Uid:\t%u\t%u\t%u\t%u\n"
        "Gid:\t%u\t%u\t%u\t%u\n"
        "VmSize:\t%u kB\n"
        "VmRSS:\t%u kB\n"
        "Threads:\t1\n"
        "Priority:\t%d\n"
        "Nice:\t0\n",
        proc->name,
        state_str,
        proc->pid,
        proc->ppid,
        proc->uid, proc->uid, proc->uid, proc->uid,
        proc->gid, proc->gid, proc->gid, proc->gid,
        (proc->heap_end - proc->code_start) / 1024,
        (proc->heap_end - proc->code_start) / 1024,
        proc->priority);
    return buf;
}

/* /proc/PID/maps */
static char* read_pid_maps(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;

    process_t* proc = process_get_by_pid(pid);
    if (!proc) { snprintf(buf, PROCFS_BUFFER_SIZE, "\n"); return buf; }

    snprintf(buf, PROCFS_BUFFER_SIZE,
        "%08x-%08x r-xp 00000000 00:00 0    [code]\n"
        "%08x-%08x rw-p 00000000 00:00 0    [data]\n"
        "%08x-%08x rw-p 00000000 00:00 0    [heap]\n"
        "%08x-%08x rw-p 00000000 00:00 0    [stack]\n",
        proc->code_start,  proc->code_end,
        proc->data_start,  proc->data_end,
        proc->heap_start,  proc->heap_end,
        proc->stack_top,   proc->stack_bottom);
    return buf;
}

/* /proc/PID/cmdline */
static char* read_pid_cmdline(uint32_t pid) {
    char* buf = (char*)malloc(256);
    if (!buf) return NULL;
    process_t* proc = process_get_by_pid(pid);
    if (!proc) { buf[0] = 0; return buf; }
    snprintf(buf, 256, "%s\n", proc->name);
    return buf;
}

/* /proc/PID/stat */
static char* read_pid_stat(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    process_t* proc = process_get_by_pid(pid);
    if (!proc) { snprintf(buf, PROCFS_BUFFER_SIZE, "\n"); return buf; }
    snprintf(buf, PROCFS_BUFFER_SIZE,
        "%u (%s) R %u 0 0 0 -1 0 0 0 0 0 "
        "%u %u 0 0 %d 0 0 0 0 %u 0\n",
        proc->pid, proc->name, proc->ppid,
        proc->jiffies_used, proc->jiffies_used,
        proc->priority,
        proc->heap_end - proc->code_start);
    return buf;
}

/* /proc/PID/fd - fájl leírók */
static char* read_pid_fd(uint32_t pid) {
    char* buf = (char*)malloc(PROCFS_BUFFER_SIZE);
    if (!buf) return NULL;
    process_t* proc = process_get_by_pid(pid);
    if (!proc) { snprintf(buf, PROCFS_BUFFER_SIZE, "\n"); return buf; }
    int pos = 0;
    for (int i = 0; i < 256; i++) {
        if (proc->fd_table[i]) {
            pos += snprintf(buf + pos, PROCFS_BUFFER_SIZE - pos,
                "%d -> fd[%d]\n", i, i);
        }
    }
    if (pos == 0) snprintf(buf, PROCFS_BUFFER_SIZE, "(no open files)\n");
    return buf;
}

/* ============================================
   PROCFS STRUKTÚRA FELÉPÍTÉS
   ============================================ */

static procfs_entry_t* alloc_entry(const char* name, uint8_t type,
                                    char* (*read_fn)(uint32_t),
                                    bool (*write_fn)(uint32_t, const char*, uint32_t)) {
    procfs_entry_t* e = (procfs_entry_t*)malloc(sizeof(procfs_entry_t));
    if (!e) return NULL;
    memset(e, 0, sizeof(procfs_entry_t));
    strncpy(e->name, name, PROCFS_MAX_NAME - 1);
    e->type = type;
    e->read_fn = read_fn;
    e->write_fn = write_fn;
    e->children = NULL;
    e->next = NULL;
    return e;
}

static void add_entry(procfs_entry_t* parent, procfs_entry_t* child) {
    if (!parent || !child) return;
    child->next = parent->children;
    parent->children = child;
}

/* PID könyvtár létrehozása */
static procfs_entry_t* create_pid_dir(uint32_t pid) {
    char name[16];
    snprintf(name, 16, "%u", pid);

    procfs_entry_t* dir = alloc_entry(name, PROCFS_TYPE_DIR, NULL, NULL);
    if (!dir) return NULL;
    dir->pid = pid;

    /* /proc/PID/status */
    procfs_entry_t* status = alloc_entry("status", PROCFS_TYPE_FILE, read_pid_status, NULL);
    if (status) { status->pid = pid; add_entry(dir, status); }

    /* /proc/PID/stat */
    procfs_entry_t* stat = alloc_entry("stat", PROCFS_TYPE_FILE, read_pid_stat, NULL);
    if (stat) { stat->pid = pid; add_entry(dir, stat); }

    /* /proc/PID/cmdline */
    procfs_entry_t* cmdline = alloc_entry("cmdline", PROCFS_TYPE_FILE, read_pid_cmdline, NULL);
    if (cmdline) { cmdline->pid = pid; add_entry(dir, cmdline); }

    /* /proc/PID/maps */
    procfs_entry_t* maps = alloc_entry("maps", PROCFS_TYPE_FILE, read_pid_maps, NULL);
    if (maps) { maps->pid = pid; add_entry(dir, maps); }

    /* /proc/PID/fd */
    procfs_entry_t* fd = alloc_entry("fd", PROCFS_TYPE_DIR, read_pid_fd, NULL);
    if (fd) { fd->pid = pid; add_entry(dir, fd); }

    return dir;
}

/* ============================================
   PROCFS FŐ FÜGGVÉNYEK
   ============================================ */

bool procfs_init(procfs_t* fs) {
    if (!fs) return false;
    memset(fs, 0, sizeof(procfs_t));

    fs->root = alloc_entry("/", PROCFS_TYPE_DIR, NULL, NULL);
    if (!fs->root) return false;

    /* Globális fájlok */
    struct { const char* name; char* (*fn)(uint32_t); } global_files[] = {
        { "version",     read_version     },
        { "uptime",      read_uptime      },
        { "cpuinfo",     read_cpuinfo     },
        { "meminfo",     read_meminfo     },
        { "stat",        read_stat        },
        { "mounts",      read_mounts      },
        { "filesystems", read_filesystems },
        { "interrupts",  read_interrupts  },
        { "loadavg",     read_loadavg     },
        { "cmdline",     read_cmdline     },
        { NULL, NULL }
    };

    for (int i = 0; global_files[i].name; i++) {
        procfs_entry_t* e = alloc_entry(global_files[i].name,
                                        PROCFS_TYPE_FILE,
                                        global_files[i].fn, NULL);
        if (e) add_entry(fs->root, e);
    }

    /* /proc/self symlink */
    procfs_entry_t* self = alloc_entry("self", PROCFS_TYPE_LINK, NULL, NULL);
    if (self) add_entry(fs->root, self);

    g_procfs = fs;
    return true;
}

bool procfs_mount(procfs_t* fs, const char* mount_point) {
    if (!fs || !mount_point) return false;
    strncpy(fs->mount_point, mount_point, PROCFS_MAX_PATH - 1);
    fs->mounted = true;

    /* PID könyvtárak hozzáadása */
    procfs_update(fs);
    return true;
}

bool procfs_umount(procfs_t* fs) {
    if (!fs) return false;
    fs->mounted = false;
    return true;
}

/* PID könyvtárak frissítése */
void procfs_update(procfs_t* fs) {
    if (!fs) return;

    /* Meglévő PID könyvtárak törlése */
    procfs_entry_t* prev = NULL;
    procfs_entry_t* cur = fs->root->children;
    while (cur) {
        procfs_entry_t* next = cur->next;
        if (cur->pid > 0) {
            if (prev) prev->next = next;
            else fs->root->children = next;
            free(cur);
        } else {
            prev = cur;
        }
        cur = next;
    }

    /* Új PID könyvtárak */
    extern process_t process_table[];
    extern uint32_t process_count;

    for (uint32_t i = 0; i < process_count; i++) {
        if (process_table[i].pid > 0 &&
            process_table[i].state != PROCESS_STATE_DEAD) {
            procfs_entry_t* pid_dir = create_pid_dir(process_table[i].pid);
            if (pid_dir) add_entry(fs->root, pid_dir);
        }
    }
}

/* Path alapján entry keresése */
static procfs_entry_t* find_entry(procfs_t* fs, const char* path, uint32_t* pid_out) {
    if (!fs || !path) return NULL;
    if (strcmp(path, "/") == 0) return fs->root;

    char tmp[PROCFS_MAX_PATH];
    strncpy(tmp, path, PROCFS_MAX_PATH - 1);

    procfs_entry_t* current = fs->root;
    char* token = strtok(tmp + 1, "/");

    while (token && current) {
        procfs_entry_t* child = current->children;
        bool found = false;
        while (child) {
            if (strcmp(child->name, token) == 0) {
                current = child;
                if (pid_out && child->pid > 0) *pid_out = child->pid;
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

/* Fájl megnyitása */
procfs_file_t* procfs_open(procfs_t* fs, const char* path) {
    if (!fs || !path) return NULL;

    uint32_t pid = 0;
    procfs_entry_t* entry = find_entry(fs, path, &pid);
    if (!entry || entry->type != PROCFS_TYPE_FILE) return NULL;

    procfs_file_t* file = (procfs_file_t*)malloc(sizeof(procfs_file_t));
    if (!file) return NULL;

    file->entry = entry;
    file->pid = pid > 0 ? pid : entry->pid;
    file->position = 0;
    file->buffer = entry->read_fn ? entry->read_fn(file->pid) : NULL;
    file->buffer_size = file->buffer ? strlen(file->buffer) : 0;

    return file;
}

/* Olvasás */
uint32_t procfs_read(procfs_file_t* file, uint8_t* buffer, uint32_t size) {
    if (!file || !buffer || !file->buffer) return 0;

    uint32_t available = file->buffer_size - file->position;
    if (available == 0) return 0;
    if (size > available) size = available;

    memcpy(buffer, file->buffer + file->position, size);
    file->position += size;

    return size;
}

/* Írás */
bool procfs_write(procfs_file_t* file, const uint8_t* buffer, uint32_t size) {
    if (!file || !buffer || !file->entry) return false;
    if (!file->entry->write_fn) return false;
    return file->entry->write_fn(file->pid, (const char*)buffer, size);
}

/* Bezárás */
bool procfs_close(procfs_file_t* file) {
    if (!file) return false;
    if (file->buffer) free(file->buffer);
    free(file);
    return true;
}

/* Könyvtár listázása */
bool procfs_list_dir(procfs_t* fs, const char* path,
                     char** entries, uint32_t* count) {
    if (!fs || !path || !entries || !count) return false;

    uint32_t pid = 0;
    procfs_entry_t* dir = find_entry(fs, path, &pid);
    if (!dir || dir->type != PROCFS_TYPE_DIR) return false;

    uint32_t n = 0;
    procfs_entry_t* child = dir->children;
    while (child && n < *count) {
        entries[n++] = child->name;
        child = child->next;
    }
    *count = n;
    return true;
}
