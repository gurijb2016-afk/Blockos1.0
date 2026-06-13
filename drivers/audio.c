#include "audio.h"
#include "timer.h"
#include "../kernel/generic.h"
static audio_t g_audio;
static void spk_play(uint32_t freq){
    uint32_t div=1193180/freq;
    outb(0x43,0xB6); outb(0x42,div&0xFF); outb(0x42,(div>>8)&0xFF);
    outb(0x61,inb(0x61)|3);
}
static void spk_stop(void){outb(0x61,inb(0x61)&~3);}
bool audio_init(void){
    memset(&g_audio,0,sizeof(g_audio));
    g_audio.sample_rate=44100; g_audio.channels=2;
    g_audio.bits=16; g_audio.volume=75; g_audio.initialized=true;
    return true;
}
void audio_play(void){g_audio.playing=true;}
void audio_stop(void){g_audio.playing=false;spk_stop();}
void audio_set_volume(uint8_t v){g_audio.volume=v;}
void audio_beep(uint32_t freq,uint32_t ms){spk_play(freq);timer_sleep(ms);spk_stop();}
bool audio_write(const uint8_t* data,uint32_t size){
    if(!g_audio.initialized) return false;
    for(uint32_t i=0;i<size;i++){
        uint32_t next=(g_audio.buf_tail+1)%AUDIO_BUFFER_SIZE;
        if(next==g_audio.buf_head) return false;
        g_audio.buffer[g_audio.buf_tail]=data[i]; g_audio.buf_tail=next;
    }
    return true;
}
