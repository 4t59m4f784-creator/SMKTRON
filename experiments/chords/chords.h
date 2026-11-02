/* SPDX-License-Identifier: GPL-3.0-only
 * CH1: global chord bank; runtime HOLD/REC/mode never restored at power-on.
 * Six reserved chord voices + six keyboard voices while in chord mode.
 * Shared audio FX and gains. One chord at a time, last pad takes precedence.
 */
#include "chord_storage.h"
typedef struct {uint8_t version,tape,count[8],notes[8][6];} ch_bank_v1_t;
typedef struct {uint8_t version,tape,count[8],notes[8][6],mix;int8_t pan_m,pan_c;} ch_bank_t;
static ch_bank_t ch_bank;
static uint32_t ch_mix_dirty,ch_mix_changed;
static uint32_t ch_hold,ch_rec,ch_target=255u,ch_active=255u,ch_pads,ch_ignore;
static uint8_t ch_capture[6],ch_n,ch_overflow,ch_seen[128],ch_prev[128];
static void ch_stop(void){uint32_t v;for(v=0;v<NV;v++)if(voice[v].group)voice[v].gate=0;ch_active=255u;redraw=0xFFu;}
static void ch_capture_reset(void){ch_n=ch_overflow=0;memset(ch_seen,0,sizeof ch_seen);memset(ch_prev,0,sizeof ch_prev);}
static int ch_commit(const ch_bank_t *next){
 ch_bank_t pending=*next;pending.mix=(uint8_t)ch_mix;pending.pan_m=(int8_t)ch_pan_m;pending.pan_c=(int8_t)ch_pan_c;
 if(ch_store_store_save(&pending,sizeof pending)){say("CHORD SAVE FAILED");return -1;}
 memcpy(&ch_bank,&pending,sizeof ch_bank);ch_mix_dirty=0;return 0;
}
static void ch_mode(uint32_t on){
 uint32_t v;
 ch_stop();ch_rec=ch_hold=0;ch_target=255u;ch_capture_reset();ch_ignore=ch_pads;
 if(ch_on!=on){for(v=0;v<NV;v++){voice[v].gate=0;voice[v].trig=voice[v].seen;voice[v].busy=0;voice[v].amp=0;}in_clear(255u);hold=pedal=0;arp_enable(0);seq_start(0);seq_rec=0;}
 ch_on=on;
 if(on){sc_on=sc_pin=0;page=PG_CHORD;}
 apply();redraw=0xFFu;say(on?"PAD CHORDS ON":"PAD CHORDS OFF");
}
static void ch_init(void){
 uint32_t i,j;ch_bank_v1_t old;
 int valid=!ch_store_store_load(&ch_bank,sizeof ch_bank)&&ch_bank.version==2u;
 if(!valid&&!ch_store_store_load(&old,sizeof old)&&old.version==1u){
  memset(&ch_bank,0,sizeof ch_bank);memcpy(&ch_bank,&old,sizeof old);
  ch_bank.version=2;ch_bank.mix=50;valid=1;
 }
 valid=valid&&ch_bank.tape<4u&&ch_bank.mix<=100u&&ch_bank.pan_m>=-100&&ch_bank.pan_m<=100&&ch_bank.pan_c>=-100&&ch_bank.pan_c<=100;
 for(i=0;i<8u&&valid;i++){if(ch_bank.count[i]>6u)valid=0;for(j=0;j<ch_bank.count[i]&&valid;j++)if(ch_bank.notes[i][j]>127u)valid=0;}
 if(!valid){memset(&ch_bank,0,sizeof ch_bank);ch_bank.version=2;ch_bank.tape=1;ch_bank.mix=50;}
 ch_mix=ch_bank.mix;ch_pan_m=ch_bank.pan_m;ch_pan_c=ch_bank.pan_c;
 ch_sound=ch_bank.tape;
}
static void ch_record(uint32_t on){ch_stop();ch_rec=on;ch_target=255u;ch_capture_reset();redraw=0xFFu;say(on?"REC: CHOOSE PAD 1-8":"CHORD REC OFF");}
static void ch_tape(uint32_t sound){if(sound==ch_sound)return;ch_bank_t b=ch_bank;b.tape=(uint8_t)sound;if(!ch_commit(&b)){ch_sound=(uint8_t)sound;say(MEL_SET_NAME[sound]);redraw=0xFFu;}}
static void ch_hold_set(uint32_t on){ch_hold=on;if(!on)ch_stop();redraw=0xFFu;say(on?"CHORD HOLD ON":"CHORD HOLD OFF");}
static int ch_pad_event(uint32_t p,uint32_t vel){
 uint32_t bit=1u<<p,was=ch_pads&bit,j;
 if(vel)ch_pads|=bit;else ch_pads&=~bit;
 if(!ch_on)return 0;
 if(ch_ignore&bit){if(!vel)ch_ignore&=~bit;return 1;}
 if(vel&&was)return 1;
 if(p<8u){
  if(vel){
   ch_stop();
   if(ch_rec){ch_target=p;ch_capture_reset();say("PLAY CHORD, RELEASE ALL");}
   else {ch_active=p;for(j=0;j<ch_bank.count[p];j++)note_on_group(ch_bank.notes[p][j],1);if(!ch_bank.count[p])say("EMPTY CHORD PAD");}
  }else if(!ch_hold&&ch_active==p)ch_stop();
 }else if(vel){
  if(p==8u)ch_record(!ch_rec);
  else if(p==9u)ch_hold_set(!ch_hold);
  else if(p>=10u&&p<=13u)ch_tape(p-10u);
  else if(p==14u){ch_record(0);ch_stop();}
  else if(p==15u)ch_mode(0);
 }
 redraw=0xFFu;return 1;
}
/* Capture original key/MIDI inputs before ARP, pedal and keyboard HOLD. Includes all 37 keys.
 * Commit only on release of all actual keys, never on pad release or sustain release.
 * More than six distinct pitches cancels the take instead of silently truncating it.
 */
static void ch_capture_poll(void){
 uint32_t n,any=0,was=0;
 if(ch_mix_dirty&&fm1_ms-ch_mix_changed>=600u){ch_bank_t b=ch_bank;if(ch_commit(&b))ch_mix_changed=fm1_ms;}
 if(!ch_on||!ch_rec||ch_target>=8u)return;
 for(n=0;n<128u;n++){
  uint32_t down=(in_get(n)&(S_KEY|S_MIDI))!=0;
  any|=down;was|=ch_prev[n];
  if(down&&!ch_prev[n]&&!ch_seen[n]){ch_seen[n]=1;if(ch_n<6u)ch_capture[ch_n++]=(uint8_t)n;else ch_overflow=1;}
  ch_prev[n]=(uint8_t)down;
 }
 if(was&&!any&&ch_n){
  ch_bank_t b=ch_bank;uint32_t i,j;uint8_t t;
  if(ch_overflow)say("MAX 6 NOTES: TRY AGAIN");
  else {
   for(i=0;i<ch_n;i++)for(j=i+1;j<ch_n;j++)if(ch_capture[j]<ch_capture[i]){t=ch_capture[i];ch_capture[i]=ch_capture[j];ch_capture[j]=t;}
   b.count[ch_target]=ch_n;memset(b.notes[ch_target],0,6);memcpy(b.notes[ch_target],ch_capture,ch_n);
   if(!ch_commit(&b)){say("CHORD SAVED GLOBALLY");ch_rec=0;ch_target=255u;}
  }
  ch_capture_reset();redraw=0xFFu;
 }
}
static void ch_mixer(uint32_t k,int32_t value){
 int32_t old=k==0?(int32_t)ch_mix:k==1?ch_pan_m:ch_pan_c,lo=k==0?0:-100;
 if(value<lo)value=lo;if(value>100)value=100;if(value==old)return;
 if(k==0)ch_mix=(uint32_t)value;else if(k==1)ch_pan_m=value;else ch_pan_c=value;
 ch_mix_dirty=1;ch_mix_changed=fm1_ms;redraw=0xFFu;
}
static void ch_knob(uint32_t k,int32_t d){
 if(k==3)ch_mode(d>0);else ch_mixer(k,(k==0?(int32_t)ch_mix:k==1?ch_pan_m:ch_pan_c)+d);
}
static void ch_tape_leds(void); /* uses SMK_PATCH_RGB after mel_smk.h defines it */
static void ch_leds(void){
 uint32_t i;for(i=0;i<16u;i++)padled_set(i,0,0,0);
 for(i=0;i<8u;i++){
  uint32_t r=ch_bank.count[i]?112u:18u,g=ch_bank.count[i]?32u:4u,b=ch_bank.count[i]?176u:28u;
  if(i==ch_active){r=255;g=48;b=144;}
  if(ch_rec&&i==ch_target){uint32_t v=(fm1_ms>>8)&1u?255u:48u;r=v;g=v/2u;b=v*208u/255u;}
  padled_set(i,r,g,b);
 }
 padled_set(8,ch_rec?255:48,0,0);padled_set(9,ch_hold?255:32,ch_hold?120:15,0);
 ch_tape_leds();
 padled_set(14,80,0,0);padled_set(15,48,48,48);
}
