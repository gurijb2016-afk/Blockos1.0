#include "../../kernel/types.h"
#include "../../kernel/generic.h"
#include "../../kernel/process.h"
extern void terminal_print(const char*);
int ps_main(int argc, char** argv){
    (void)argc;(void)argv;
    terminal_print("  PID  PPID  STATE       NAME\n");
    extern process_t process_table[]; extern uint32_t process_count;
    for(uint32_t i=0;i<process_count;i++){
        process_t* p=&process_table[i];
        if(p->state==PROCESS_STATE_DEAD) continue;
        const char* st="?";
        switch(p->state){case PROCESS_STATE_READY:st="running";break;case PROCESS_STATE_BLOCKED:st="sleeping";break;case PROCESS_STATE_ZOMBIE:st="zombie";break;}
        char buf[64]; snprintf(buf,64,"%6u %5u  %-10s %s\n",p->pid,p->ppid,st,p->name);
        terminal_print(buf);
    }
    return 0;
}
