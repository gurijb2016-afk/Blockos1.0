#ifndef BLOCKPKG_H
#define BLOCKPKG_H
#include "../../kernel/types.h"
#define PKG_MAX_PACKAGES 1024
#define PKG_MAX_NAME     64
#define PKG_STATE_NONE      0
#define PKG_STATE_INSTALLED 1
#define PKG_STATE_BROKEN    2
typedef struct {
    char     name[PKG_MAX_NAME];
    char     version[32];
    char     description[256];
    uint32_t size;
    uint8_t  state;
    char     deps[16][PKG_MAX_NAME];
    uint32_t dep_count;
} package_t;
bool     pkgmgr_init(void);
bool     pkgmgr_update(void);
bool     pkgmgr_install(const char* name);
bool     pkgmgr_remove(const char* name);
package_t* pkgmgr_find(const char* name);
uint32_t pkgmgr_list_installed(package_t** list, uint32_t max);
void     pkgmgr_stats(uint32_t* total, uint32_t* installed, uint32_t* size);
#endif
