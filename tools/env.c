#include "../../kernel/types.h"
#include "../../kernel/generic.h"
extern void terminal_print(const char*);
#define ENV_MAX 64
static char g_env_keys[ENV_MAX][64];
static char g_env_vals[ENV_MAX][256];
static int  g_env_count=0;
void env_set(const char* k, const char* v) {
    for(int i=0;i<g_env_count;i++) if(strcmp(g_env_keys[i],k)==0){strncpy(g_env_vals[i],v,255);return;}
    if(g_env_count<ENV_MAX){strncpy(g_env_keys[g_env_count],k,63);strncpy(g_env_vals[g_env_count],v,255);g_env_count++;}
}
const char* env_get(const char* k) {
    for(int i=0;i<g_env_count;i++) if(strcmp(g_env_keys[i],k)==0) return g_env_vals[i];
    return NULL;
}
int env_main(int argc, char** argv) {
    (void)argc;(void)argv;
    for(int i=0;i<g_env_count;i++){terminal_print(g_env_keys[i]);terminal_print("=");terminal_print(g_env_vals[i]);terminal_print("\n");}
    return 0;
}
