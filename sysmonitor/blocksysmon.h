#ifndef BLOCKSYSMON_H
#define BLOCKSYSMON_H
#include "../../kernel/types.h"
bool sysmon_init(void);
void sysmon_update(void);
void sysmon_draw(void);
#endif
