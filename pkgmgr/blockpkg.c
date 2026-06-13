#include "blockpkg.h"
#include "../../kernel/generic.h"

static package_t g_packages[PKG_MAX_PACKAGES];
static uint32_t  g_count = 0;
static uint32_t  g_installed = 0;

bool pkgmgr_init(void) {
    memset(g_packages, 0, sizeof(g_packages));
    return true;
}

package_t* pkgmgr_find(const char* name) {
    for (uint32_t i = 0; i < g_count; i++)
        if (strcmp(g_packages[i].name, name) == 0)
            return &g_packages[i];
    return NULL;
}

bool pkgmgr_install(const char* name) {
    package_t* pkg = pkgmgr_find(name);
    if (!pkg) return false;
    if (pkg->state == PKG_STATE_INSTALLED) return true;
    for (uint32_t i = 0; i < pkg->dep_count; i++)
        pkgmgr_install(pkg->deps[i]);
    pkg->state = PKG_STATE_INSTALLED;
    g_installed++;
    return true;
}

bool pkgmgr_remove(const char* name) {
    package_t* pkg = pkgmgr_find(name);
    if (!pkg || pkg->state != PKG_STATE_INSTALLED) return false;
    pkg->state = PKG_STATE_NONE;
    g_installed--;
    return true;
}

bool pkgmgr_update(void) { return true; }

uint32_t pkgmgr_list_installed(package_t** list, uint32_t max) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < g_count && n < max; i++)
        if (g_packages[i].state == PKG_STATE_INSTALLED)
            list[n++] = &g_packages[i];
    return n;
}

void pkgmgr_stats(uint32_t* total, uint32_t* installed, uint32_t* size) {
    *total = g_count; *installed = g_installed; *size = 0;
    for (uint32_t i = 0; i < g_count; i++)
        if (g_packages[i].state == PKG_STATE_INSTALLED)
            *size += g_packages[i].size;
}
