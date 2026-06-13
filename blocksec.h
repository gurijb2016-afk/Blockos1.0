#ifndef BLOCKSEC_H
#define BLOCKSEC_H
#include "../kernel/types.h"
#include "../kernel/process.h"

/* BlockSEC - BlockOS Biztonsági Rendszer
   SELinux-szerű MAC (Mandatory Access Control) */

#define BLOCKSEC_VERSION    "1.0"
#define BLOCKSEC_MAX_LABELS  256
#define BLOCKSEC_MAX_RULES   1024
#define BLOCKSEC_MAX_DOMAINS 64
#define BLOCKSEC_MAX_TYPES   128
#define BLOCKSEC_LOG_SIZE    512

/* Biztonsági módok */
#define BLOCKSEC_MODE_DISABLED   0  /* Ki */
#define BLOCKSEC_MODE_PERMISSIVE 1  /* Engedékeny - csak naplóz */
#define BLOCKSEC_MODE_ENFORCING  2  /* Szigorú - blokkol */

/* Hozzáférési jogok */
#define BLOCKSEC_PERM_READ     (1 << 0)
#define BLOCKSEC_PERM_WRITE    (1 << 1)
#define BLOCKSEC_PERM_EXEC     (1 << 2)
#define BLOCKSEC_PERM_CREATE   (1 << 3)
#define BLOCKSEC_PERM_DELETE   (1 << 4)
#define BLOCKSEC_PERM_OPEN     (1 << 5)
#define BLOCKSEC_PERM_IOCTL    (1 << 6)
#define BLOCKSEC_PERM_CONNECT  (1 << 7)
#define BLOCKSEC_PERM_BIND     (1 << 8)
#define BLOCKSEC_PERM_SEND     (1 << 9)
#define BLOCKSEC_PERM_RECV     (1 << 10)
#define BLOCKSEC_PERM_FORK     (1 << 11)
#define BLOCKSEC_PERM_KILL     (1 << 12)
#define BLOCKSEC_PERM_ALL      0xFFFFFFFF

/* Objektum osztályok */
#define BLOCKSEC_CLASS_FILE      0
#define BLOCKSEC_CLASS_DIR       1
#define BLOCKSEC_CLASS_PROCESS   2
#define BLOCKSEC_CLASS_SOCKET    3
#define BLOCKSEC_CLASS_DEVICE    4
#define BLOCKSEC_CLASS_PIPE      5
#define BLOCKSEC_CLASS_SHM       6
#define BLOCKSEC_CLASS_KERNEL    7

/* Biztonsági label */
typedef struct {
    char     user[32];      /* blocksec_u */
    char     role[32];      /* blocksec_r */
    char     type[32];      /* blocksec_t */
    uint32_t level;         /* MLS szint */
} sec_label_t;

/* Hozzáférési szabály */
typedef struct {
    char     source_type[32];  /* Ki kér hozzáférést */
    char     target_type[32];  /* Mihez kér hozzáférést */
    uint8_t  object_class;     /* Objektum osztály */
    uint32_t permissions;      /* Engedélyezett műveletek */
    bool     allow;            /* Engedély vagy tiltás */
    char     name[64];         /* Szabály neve */
} sec_rule_t;

/* Domain - folyamat biztonsági kontextus */
typedef struct {
    char       name[32];
    sec_label_t label;
    uint32_t   allowed_perms;
    bool       privileged;
    bool       confined;       /* Korlátozott-e */
} sec_domain_t;

/* Napló bejegyzés */
typedef struct {
    uint64_t   timestamp;
    uint32_t   pid;
    char       source[32];
    char       target[32];
    uint8_t    class;
    uint32_t   perm;
    bool       allowed;
    char       message[128];
} sec_log_t;

/* BlockSEC fő struktúra */
typedef struct {
    uint8_t     mode;
    sec_rule_t  rules[BLOCKSEC_MAX_RULES];
    uint32_t    rule_count;
    sec_domain_t domains[BLOCKSEC_MAX_DOMAINS];
    uint32_t    domain_count;
    sec_log_t   log[BLOCKSEC_LOG_SIZE];
    uint32_t    log_head;
    uint32_t    log_count;
    uint32_t    denied_count;
    uint32_t    allowed_count;
} blocksec_t;

/* Inicializálás */
bool blocksec_init(blocksec_t* sec);
void blocksec_set_mode(blocksec_t* sec, uint8_t mode);
uint8_t blocksec_get_mode(blocksec_t* sec);

/* Szabályok */
bool blocksec_add_rule(blocksec_t* sec, const char* src, const char* tgt,
                       uint8_t class, uint32_t perms, bool allow, const char* name);
bool blocksec_remove_rule(blocksec_t* sec, const char* name);
void blocksec_load_default_policy(blocksec_t* sec);
bool blocksec_load_policy_file(blocksec_t* sec, const char* path);

/* Label kezelés */
bool blocksec_set_process_label(blocksec_t* sec, uint32_t pid, sec_label_t* label);
bool blocksec_get_process_label(blocksec_t* sec, uint32_t pid, sec_label_t* out);
bool blocksec_set_file_label(blocksec_t* sec, const char* path, sec_label_t* label);

/* Hozzáférés ellenőrzés */
bool blocksec_check(blocksec_t* sec, uint32_t pid, const char* target,
                    uint8_t class, uint32_t perm);
bool blocksec_check_file(blocksec_t* sec, uint32_t pid, const char* path, uint32_t perm);
bool blocksec_check_process(blocksec_t* sec, uint32_t src_pid, uint32_t dst_pid, uint32_t perm);
bool blocksec_check_network(blocksec_t* sec, uint32_t pid, uint32_t perm);

/* Domain kezelés */
int  blocksec_add_domain(blocksec_t* sec, const char* name, bool privileged, bool confined);
sec_domain_t* blocksec_get_domain(blocksec_t* sec, const char* name);
bool blocksec_transition(blocksec_t* sec, uint32_t pid, const char* new_domain);

/* Napló */
void blocksec_log(blocksec_t* sec, uint32_t pid, const char* src,
                  const char* tgt, uint8_t class, uint32_t perm, bool allowed);
uint32_t blocksec_get_log(blocksec_t* sec, sec_log_t* out, uint32_t max);
void blocksec_print_stats(blocksec_t* sec);

/* XML policy betöltés */
bool blocksec_load_xml_policy(blocksec_t* sec, const char* xml_path);

#endif
