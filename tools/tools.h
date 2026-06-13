#ifndef TOOLS_H
#define TOOLS_H
#include "../../kernel/types.h"

/* Tool main függvények */
int ls_main(int argc, char** argv);
int cp_main(int argc, char** argv);
int mv_main(int argc, char** argv);
int rm_main(int argc, char** argv);
int mkdir_main(int argc, char** argv);
int cat_main(int argc, char** argv);
int echo_main(int argc, char** argv);
int pwd_main(int argc, char** argv);
int ps_main(int argc, char** argv);
int kill_main(int argc, char** argv);
int free_main(int argc, char** argv);
int uname_main(int argc, char** argv);
int touch_main(int argc, char** argv);
int grep_main(int argc, char** argv);
int head_main(int argc, char** argv);
int tail_main(int argc, char** argv);
int wc_main(int argc, char** argv);
int sort_main(int argc, char** argv);
int date_main(int argc, char** argv);
int df_main(int argc, char** argv);
int du_main(int argc, char** argv);
int chmod_main(int argc, char** argv);
int ln_main(int argc, char** argv);
int stat_main(int argc, char** argv);
int find_main(int argc, char** argv);
int clear_main(int argc, char** argv);
int history_main(int argc, char** argv);
int env_main(int argc, char** argv);

/* Dispatcher */
typedef struct {
    const char* name;
    int (*main)(int, char**);
    const char* desc;
} tool_t;

static const tool_t g_tools[] = {
    {"ls",      ls_main,      "Könyvtár listázás"},
    {"cp",      cp_main,      "Fájl másolás"},
    {"mv",      mv_main,      "Fájl mozgatás"},
    {"rm",      rm_main,      "Fájl törlés"},
    {"mkdir",   mkdir_main,   "Könyvtár létrehozás"},
    {"cat",     cat_main,     "Fájl tartalom"},
    {"echo",    echo_main,    "Szöveg kiírás"},
    {"pwd",     pwd_main,     "Aktuális könyvtár"},
    {"ps",      ps_main,      "Folyamatok listája"},
    {"kill",    kill_main,    "Folyamat leállítás"},
    {"free",    free_main,    "Memória állapot"},
    {"uname",   uname_main,   "Rendszer info"},
    {"touch",   touch_main,   "Fájl létrehozás"},
    {"grep",    grep_main,    "Szöveg keresés"},
    {"head",    head_main,    "Fájl eleje"},
    {"tail",    tail_main,    "Fájl vége"},
    {"wc",      wc_main,      "Szó/sor számláló"},
    {"sort",    sort_main,    "Rendezés"},
    {"date",    date_main,    "Dátum/idő"},
    {"df",      df_main,      "Lemez foglaltság"},
    {"du",      du_main,      "Könyvtár méret"},
    {"chmod",   chmod_main,   "Jogosultság"},
    {"ln",      ln_main,      "Link létrehozás"},
    {"stat",    stat_main,    "Fájl statisztika"},
    {"find",    find_main,    "Fájl keresés"},
    {"clear",   clear_main,   "Képernyő törlés"},
    {"history", history_main, "Parancs előzmény"},
    {"env",     env_main,     "Környezeti változók"},
    {NULL, NULL, NULL}
};

int tools_dispatch(const char* cmd, int argc, char** argv);
#endif
