#include "blocksec.h"
#include "../kernel/generic.h"
#include "../fs/vfs.h"

static blocksec_t* g_sec = NULL;

/* ============================================
   INICIALIZÁLÁS
   ============================================ */

bool blocksec_init(blocksec_t* sec) {
    if (!sec) return false;
    memset(sec, 0, sizeof(blocksec_t));
    sec->mode = BLOCKSEC_MODE_PERMISSIVE;
    g_sec = sec;

    /* Alapértelmezett policy betöltés */
    blocksec_load_default_policy(sec);
    return true;
}

void blocksec_set_mode(blocksec_t* sec, uint8_t mode) {
    if (!sec) return;
    sec->mode = mode;
    const char* modes[] = {"disabled","permissive","enforcing"};
    char buf[64];
    snprintf(buf, 64, "BlockSEC: mód váltás -> %s\n", modes[mode < 3 ? mode : 0]);
}

uint8_t blocksec_get_mode(blocksec_t* sec) {
    return sec ? sec->mode : BLOCKSEC_MODE_DISABLED;
}

/* ============================================
   ALAPÉRTELMEZETT POLICY
   ============================================ */

void blocksec_load_default_policy(blocksec_t* sec) {
    if (!sec) return;

    /* Domének */
    blocksec_add_domain(sec, "kernel_t",   true,  false);
    blocksec_add_domain(sec, "init_t",     true,  false);
    blocksec_add_domain(sec, "user_t",     false, true);
    blocksec_add_domain(sec, "shell_t",    false, true);
    blocksec_add_domain(sec, "gui_t",      false, true);
    blocksec_add_domain(sec, "network_t",  false, true);
    blocksec_add_domain(sec, "untrusted_t",false, true);

    /* Szabályok */

    /* Kernel mindent tud */
    blocksec_add_rule(sec, "kernel_t", "any_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_ALL, true, "kernel_all");

    /* Init alapvető jogok */
    blocksec_add_rule(sec, "init_t", "system_file_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_READ|BLOCKSEC_PERM_EXEC, true, "init_read_sys");

    /* User fájl olvasás */
    blocksec_add_rule(sec, "user_t", "user_home_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_READ|BLOCKSEC_PERM_WRITE|BLOCKSEC_PERM_CREATE, true, "user_home_rw");

    /* User NEM írhat rendszerfájlba */
    blocksec_add_rule(sec, "user_t", "system_file_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_WRITE|BLOCKSEC_PERM_DELETE, false, "user_no_sys_write");

    /* Shell */
    blocksec_add_rule(sec, "shell_t", "user_home_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_READ|BLOCKSEC_PERM_EXEC, true, "shell_exec");

    /* GUI olvashat mindent */
    blocksec_add_rule(sec, "gui_t", "any_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_READ, true, "gui_read_all");

    /* Network folyamat */
    blocksec_add_rule(sec, "network_t", "any_t",
        BLOCKSEC_CLASS_SOCKET, BLOCKSEC_PERM_CONNECT|BLOCKSEC_PERM_SEND|BLOCKSEC_PERM_RECV, true, "net_socket");

    /* Untrusted - nagyon korlátozott */
    blocksec_add_rule(sec, "untrusted_t", "user_home_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_READ, true, "untrusted_read_home");
    blocksec_add_rule(sec, "untrusted_t", "system_file_t",
        BLOCKSEC_CLASS_FILE, BLOCKSEC_PERM_ALL, false, "untrusted_no_sys");
    blocksec_add_rule(sec, "untrusted_t", "any_t",
        BLOCKSEC_CLASS_PROCESS, BLOCKSEC_PERM_FORK|BLOCKSEC_PERM_KILL, false, "untrusted_no_proc");
}

/* ============================================
   SZABÁLYOK
   ============================================ */

bool blocksec_add_rule(blocksec_t* sec, const char* src, const char* tgt,
                        uint8_t class, uint32_t perms, bool allow, const char* name) {
    if (!sec || sec->rule_count >= BLOCKSEC_MAX_RULES) return false;
    sec_rule_t* r = &sec->rules[sec->rule_count++];
    strncpy(r->source_type, src,  31);
    strncpy(r->target_type, tgt,  31);
    strncpy(r->name,        name, 63);
    r->object_class = class;
    r->permissions  = perms;
    r->allow        = allow;
    return true;
}

bool blocksec_remove_rule(blocksec_t* sec, const char* name) {
    if (!sec) return false;
    for (uint32_t i = 0; i < sec->rule_count; i++) {
        if (strcmp(sec->rules[i].name, name) == 0) {
            for (uint32_t j = i; j < sec->rule_count-1; j++)
                sec->rules[j] = sec->rules[j+1];
            sec->rule_count--;
            return true;
        }
    }
    return false;
}

/* ============================================
   HOZZÁFÉRÉS ELLENŐRZÉS
   ============================================ */

bool blocksec_check(blocksec_t* sec, uint32_t pid, const char* target,
                     uint8_t class, uint32_t perm) {
    if (!sec || sec->mode == BLOCKSEC_MODE_DISABLED) return true;

    /* Folyamat label lekérés */
    sec_label_t label;
    if (!blocksec_get_process_label(sec, pid, &label)) {
        /* Ismeretlen folyamat - alapértelmezett tiltás enforcing módban */
        if (sec->mode == BLOCKSEC_MODE_ENFORCING) return false;
        return true;
    }

    /* Szabályok keresése */
    bool found_allow = false;
    bool found_deny  = false;

    for (uint32_t i = 0; i < sec->rule_count; i++) {
        sec_rule_t* r = &sec->rules[i];

        /* Source type egyezés */
        bool src_match = (strcmp(r->source_type, label.type) == 0 ||
                          strcmp(r->source_type, "any_t") == 0);
        /* Target type egyezés */
        bool tgt_match = (strcmp(r->target_type, target) == 0 ||
                          strcmp(r->target_type, "any_t") == 0);

        if (!src_match || !tgt_match) continue;
        if (r->object_class != class) continue;
        if (!(r->permissions & perm)) continue;

        if (r->allow)  found_allow = true;
        else           found_deny  = true;
    }

    /* Tiltás prioritás */
    bool allowed = found_deny ? false : found_allow;

    /* Napló */
    blocksec_log(sec, pid, label.type, target, class, perm, allowed);

    if (!allowed) {
        sec->denied_count++;
        if (sec->mode == BLOCKSEC_MODE_PERMISSIVE) return true;
        return false;
    }

    sec->allowed_count++;
    return true;
}

bool blocksec_check_file(blocksec_t* sec, uint32_t pid, const char* path, uint32_t perm) {
    /* Path alapján típus meghatározás */
    const char* type = "any_t";
    if (strncmp(path, "/home", 5) == 0)  type = "user_home_t";
    if (strncmp(path, "/sys",  4) == 0)  type = "system_file_t";
    if (strncmp(path, "/proc", 5) == 0)  type = "proc_t";
    if (strncmp(path, "/dev",  4) == 0)  type = "device_t";
    if (strncmp(path, "/etc",  4) == 0)  type = "config_file_t";
    if (strncmp(path, "/bin",  4) == 0)  type = "system_file_t";
    return blocksec_check(sec, pid, type, BLOCKSEC_CLASS_FILE, perm);
}

bool blocksec_check_process(blocksec_t* sec, uint32_t src, uint32_t dst, uint32_t perm) {
    char dst_str[16];
    snprintf(dst_str, 16, "pid_%u", dst);
    return blocksec_check(sec, src, dst_str, BLOCKSEC_CLASS_PROCESS, perm);
}

bool blocksec_check_network(blocksec_t* sec, uint32_t pid, uint32_t perm) {
    return blocksec_check(sec, pid, "network_t", BLOCKSEC_CLASS_SOCKET, perm);
}

/* ============================================
   DOMAIN KEZELÉS
   ============================================ */

int blocksec_add_domain(blocksec_t* sec, const char* name, bool privileged, bool confined) {
    if (!sec || sec->domain_count >= BLOCKSEC_MAX_DOMAINS) return -1;
    sec_domain_t* d = &sec->domains[sec->domain_count];
    strncpy(d->name, name, 31);
    strncpy(d->label.type, name, 31);
    strncpy(d->label.user, "blocksec_u", 31);
    strncpy(d->label.role, privileged ? "system_r" : "user_r", 31);
    d->privileged = privileged;
    d->confined   = confined;
    d->allowed_perms = privileged ? BLOCKSEC_PERM_ALL : 0;
    return sec->domain_count++;
}

sec_domain_t* blocksec_get_domain(blocksec_t* sec, const char* name) {
    if (!sec) return NULL;
    for (uint32_t i = 0; i < sec->domain_count; i++)
        if (strcmp(sec->domains[i].name, name) == 0)
            return &sec->domains[i];
    return NULL;
}

/* Process label tárolás */
static sec_label_t g_proc_labels[256];
static uint32_t    g_proc_pids[256];
static uint32_t    g_proc_label_count = 0;

bool blocksec_set_process_label(blocksec_t* sec, uint32_t pid, sec_label_t* label) {
    if (!sec || !label) return false;
    /* Meglévő frissítése */
    for (uint32_t i = 0; i < g_proc_label_count; i++) {
        if (g_proc_pids[i] == pid) { g_proc_labels[i] = *label; return true; }
    }
    /* Új hozzáadása */
    if (g_proc_label_count >= 256) return false;
    g_proc_pids[g_proc_label_count]   = pid;
    g_proc_labels[g_proc_label_count] = *label;
    g_proc_label_count++;
    return true;
}

bool blocksec_get_process_label(blocksec_t* sec, uint32_t pid, sec_label_t* out) {
    if (!sec || !out) return false;
    for (uint32_t i = 0; i < g_proc_label_count; i++) {
        if (g_proc_pids[i] == pid) { *out = g_proc_labels[i]; return true; }
    }
    return false;
}

bool blocksec_transition(blocksec_t* sec, uint32_t pid, const char* new_domain) {
    sec_domain_t* d = blocksec_get_domain(sec, new_domain);
    if (!d) return false;
    return blocksec_set_process_label(sec, pid, &d->label);
}

/* ============================================
   NAPLÓ
   ============================================ */

void blocksec_log(blocksec_t* sec, uint32_t pid, const char* src,
                   const char* tgt, uint8_t class, uint32_t perm, bool allowed) {
    if (!sec) return;
    sec_log_t* entry = &sec->log[sec->log_head % BLOCKSEC_LOG_SIZE];
    entry->timestamp = kernel_get_ticks();
    entry->pid       = pid;
    entry->class     = class;
    entry->perm      = perm;
    entry->allowed   = allowed;
    strncpy(entry->source, src, 31);
    strncpy(entry->target, tgt, 31);

    const char* class_names[] = {"file","dir","process","socket","device","pipe","shm","kernel"};
    snprintf(entry->message, 127,
        "%s: avc: %s { %s%s%s%s } pid=%u scontext=%s tcontext=%s tclass=%s",
        allowed ? "ALLOW" : "DENY",
        allowed ? "allowed" : "denied",
        (perm & BLOCKSEC_PERM_READ)  ? "read " : "",
        (perm & BLOCKSEC_PERM_WRITE) ? "write " : "",
        (perm & BLOCKSEC_PERM_EXEC)  ? "exec " : "",
        (perm & BLOCKSEC_PERM_CREATE)? "create " : "",
        pid, src, tgt,
        class < 8 ? class_names[class] : "unknown");

    sec->log_head++;
    if (sec->log_count < BLOCKSEC_LOG_SIZE) sec->log_count++;
}

uint32_t blocksec_get_log(blocksec_t* sec, sec_log_t* out, uint32_t max) {
    if (!sec || !out) return 0;
    uint32_t n = sec->log_count < max ? sec->log_count : max;
    for (uint32_t i = 0; i < n; i++)
        out[i] = sec->log[(sec->log_head - n + i) % BLOCKSEC_LOG_SIZE];
    return n;
}

void blocksec_print_stats(blocksec_t* sec) {
    if (!sec) return;
    char buf[256];
    const char* modes[] = {"disabled","permissive","enforcing"};
    snprintf(buf, 256,
        "BlockSEC Statisztika:\n"
        "  Mód:      %s\n"
        "  Szabályok:%u\n"
        "  Domének:  %u\n"
        "  Engedve:  %u\n"
        "  Tiltva:   %u\n",
        modes[sec->mode < 3 ? sec->mode : 0],
        sec->rule_count,
        sec->domain_count,
        sec->allowed_count,
        sec->denied_count);
}

/* XML policy betöltés */
bool blocksec_load_xml_policy(blocksec_t* sec, const char* xml_path) {
    if (!sec || !xml_path) return false;
    int fd = vfs_open(xml_path, VFS_O_RDONLY);
    if (fd < 0) return false;
    /* XML parse egyszerűsített implementáció */
    uint8_t buf[4096]; uint32_t n;
    while ((n = vfs_read(fd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = 0;
        /* allow rule keresés */
        char* pos = (char*)buf;
        while ((pos = strstr(pos, "<allow"))) {
            /* Egyszerű attribútum parse */
            pos++;
        }
    }
    vfs_close(fd);
    return true;
}

bool blocksec_load_policy_file(blocksec_t* sec, const char* path) {
    return blocksec_load_xml_policy(sec, path);
}
