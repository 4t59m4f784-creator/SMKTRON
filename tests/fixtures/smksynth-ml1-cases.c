/* SPDX-License-Identifier: GPL-3.0-only */
#define main obv_host_main
#include "../firmware/src/obervave.c"
#undef main
#include <assert.h>
static unsigned observed,ons;
static uint8_t wire[16][128],sound[16][128],pedal[16];
static void collect(void){assert(host_midi_count-observed<=256);while(observed<host_midi_count){unsigned i=observed++%256,st=host_midi_sent[i][0],c=st&15,a=host_midi_sent[i][1],b=host_midi_sent[i][2];if((st&240)==144&&b){assert(!wire[c][a]);wire[c][a]=sound[c][a]=1;ons++;}else if((st&240)==128||((st&240)==144&&!b)){wire[c][a]=0;if(!pedal[c])sound[c][a]=0;}else if((st&240)==176){if(a==64){pedal[c]=b>=64;if(!pedal[c])for(unsigned n=0;n<128;n++)if(!wire[c][n])sound[c][n]=0;}if(a==120||a==123){memset(wire[c],0,128);if(a==120||!pedal[c])memset(sound[c],0,128);}}}}
static void tick(void){fm1_ms++;mm_service();collect();}
static void drain(void){for(unsigned n=0;n<300;n++)tick();}
static void input(void){fm1_ms++;mm_input();mm_service();collect();}
static void key(unsigned k,unsigned down){if(down)fm1_in.keys|=(uint64_t)1<<k;else fm1_in.keys&=~((uint64_t)1<<k);fm1_in.key_vel[k]=103;input();}
static void pad(unsigned p,unsigned v){fm1_in.pad_st[p]=v?128:0;host_qput(0,p<<8|v);input();}
static void route(unsigned m){mm_pbank=1;for(unsigned c=0;c<16;c++)if(m>>c&1)pad(c,100);for(unsigned c=0;c<16;c++)if(m>>c&1)pad(c,0);drain();assert(mm_key_channels==m&&!mm_channel_held);}
static void quiet(void){for(unsigned c=0;c<16;c++)for(unsigned n=0;n<128;n++)assert(!wire[c][n]&&!sound[c][n]);}
int main(void){engine_init();pf_init();pat_init(&pat);mm_boot();fm1_ms=1000;fm1_in.frames=400;fm1_in.wheel[0]=535;fm1_in.wheel[1]=620;mm_enter();drain();mm_wheel_prev[0]=8192;mm_wheel_prev[1]=0;
 mm_entry_t saved[16];memcpy(saved,&mm_profile()->map[40],sizeof saved);
 unsigned n=ons;route(4);assert(ons==n);key(12,1);drain();assert(wire[2][60]&&!wire[0][60]);key(12,0);drain();quiet();
 /* Old configurable B mappings are ignored, including CC toggle mappings. */
 mm_profile()->map[40]=(mm_entry_t){MM_TOGGLE,77,16};unsigned start=host_midi_count;route(1);for(unsigned i=start;i<host_midi_count;i++)assert(!((host_midi_sent[i%256][0]&240)==176&&host_midi_sent[i%256][1]==77));mm_profile()->map[40]=saved[0];
 route(37);mm_leds();assert(padled_rgb[0]==padled_rgb[2]&&padled_rgb[2]==padled_rgb[5]&&padled_rgb[1]!=padled_rgb[0]);for(unsigned k=0;k<3;k++)key(k*12,1);drain();for(unsigned c=0;c<16;c++)for(unsigned k=0;k<3;k++)assert(wire[c][48+k*12]==(c==0||c==2||c==5));
 route(8);key(3,1);drain();assert(wire[3][51]);for(unsigned k=0;k<3;k++)key(k*12,0);drain();assert(wire[3][51]&&!wire[0][48]&&!wire[2][60]&&!wire[5][72]);key(3,0);drain();quiet();
 mm_pbank=0;assert(mm_key_channels==8);pad(8,111);drain();assert(wire[9][36]);pad(8,0);drain();quiet();
 route(5);key(12,1);drain();n=ons;mm_oct=1;key(0,1);drain();assert(ons==n);key(12,0);drain();assert(wire[0][60]&&wire[2][60]);key(0,0);drain();quiet();mm_oct=0;
 host_qput(0,4<<8|100);fm1_in.pad_st[4]=128;fm1_in.keys|=(uint64_t)1<<12;input();drain();assert(mm_key_channels==16&&wire[4][60]);pad(4,0);key(12,0);drain();quiet();
 route(5);key(12,1);drain();mm_pedal_event(1);drain();key(12,0);drain();assert(sound[0][60]&&sound[2][60]);route(64);key(14,1);drain();assert(pedal[6]);key(14,0);drain();assert(sound[6][62]);mm_pedal_event(0);drain();quiet();assert(!pedal[0]&&!pedal[2]&&!pedal[6]);
 mm_wheel_prev[0]=16383;mm_wheel_prev[1]=91;route(5);mm_profile()->map[MM_PITCH].ch=16;mm_profile()->map[MM_MOD].ch=15;mm_bend_value=15001;mm_bend_pending=1;mm_bend_channels=5;mm_cv(MM_MOD,109,0);start=host_midi_count;drain();unsigned bend=0,mod=0;for(unsigned i=start;i<host_midi_count;i++){unsigned j=i%256,st=host_midi_sent[j][0];assert((st&15)==0||(st&15)==2);if((st&240)==224){assert((host_midi_sent[j][2]<<7|host_midi_sent[j][1])==15001);bend++;}if((st&240)==176&&host_midi_sent[j][1]==1){assert(host_midi_sent[j][2]==109);mod++;}}assert(bend==2&&mod==2);
 mm_profile()->map[0].ch=9;start=host_midi_count;mm_cv(0,87,0);drain();assert(host_midi_count==start+1&&host_midi_sent[start%256][0]==0xb8);
 route(65535);n=ons;for(unsigned k=0;k<37;k++)fm1_in.key_vel[k]=103;fm1_in.keys=((uint64_t)1<<37)-1;input();drain();assert(ons==n+592&&mm_w==mm_r&&!mm_panic_left);for(unsigned c=0;c<16;c++)for(unsigned j=48;j<=84;j++)assert(wire[c][j]);
 host_midi_blocked=1;fm1_in.keys=0;input();route(128);mm_oct=2;key(0,1);assert(mm_key_sent[0]&&mm_key_target[0]==128);host_midi_blocked=0;drain();for(unsigned c=0;c<16;c++)for(unsigned j=0;j<128;j++)assert(wire[c][j]==(c==7&&j==72));key(0,0);drain();quiet();mm_oct=0;
 route(5);host_midi_blocked=1;n=ons;key(12,1);key(12,0);host_midi_blocked=0;drain();assert(ons==n);quiet();for(unsigned i=0;i<200;i++){key(12,1);if(i%2){host_midi_blocked=1;key(12,0);key(12,1);host_midi_blocked=0;}key(12,0);tick();}drain();quiet();assert(!mm_panic_left);
 route(65535);fm1_in.keys=((uint64_t)1<<37)-1;input();drain();mm_pedal_event(1);drain();mm_safety();drain();quiet();for(unsigned c=0;c<16;c++)assert(!pedal[c]);assert(!mm_panic_left);fm1_in.keys=0;input();key(12,1);drain();mm_leave();drain();quiet();assert(!mm_on&&!mm_panic_left);assert(!memcmp(saved,&mm_profile()->map[40],sizeof saved)&&mm_valid(&mm_data));
 puts("PASS ML1: selector/chord gestures, no B notes/CCs, solo replacement, original note destinations, shared pitches, same-frame selection, sustain history, full bend/mod fan-out, independent control channels, 37x16, backpressure/retrigger, panic/exit and unchanged saved maps.");}
