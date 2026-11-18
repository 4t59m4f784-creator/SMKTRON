/* SPDX-License-Identifier: GPL-3.0-only
 * Mapping conventions adapted from Snoop37/smk_daw.h:
 * Copyright (C) 2026 M. Geisler (genkigenki/makkuro).
 * SMKSYNTH: four on-device profiles, custom colours and exclusive controller.
 * Main context only. Continuous controls coalesce; releases have priority.
 */
#pragma once
#include "smk_wheel.h"
#define MM_N 71u
#define MM_PAD 24u
#define MM_BTN 56u
#define MM_PITCH 69u
#define MM_MOD 70u
#define MM_KEYS 71u
#define MM_SOURCES 109u /* 71 controls + 37 keys + pedal */
enum { MM_OFF, MM_CC, MM_TOGGLE, MM_NOTE, MM_BEND, MM_TYPES };
static const char *const MM_TYPE_NAME[]={"OFF","CC","CC TOGGLE","NOTE","PITCH BEND"};
static const char *const MM_COLOR_NAME[]={"AUTO","RED","ORANGE","YELLOW","GREEN","CYAN","BLUE","VIOLET","PINK","WHITE"};
static const uint8_t MM_COLOR[][3]={{0,0,0},{255,40,48},{255,105,20},{255,198,24},{30,204,112},{20,210,230},{40,124,255},{160,65,255},{255,65,160},{235,235,235}};
static const uint8_t MM_BUTTON[13]={BTN_LEFT,BTN_RIGHT,BTN_PLAY,BTN_REC,BTN_SEQ,BTN_FX,BTN_EDIT,BTN_ARP,BTN_LFO,BTN_SEL,BTN_ENV,BTN_GLO,BTN_SAVE};
static const char *const MM_BUTTON_NAME[13]={"LEFT","RIGHT","PLAY","REC","SEQ","FX","PARA","ARP","REPEAT","SCALE","CHORD","GLOBE","BT"};
static const uint8_t MM_BUTTON_CC[13]={110,111,115,117,112,113,114,118,119,85,86,87,88};
typedef struct {uint8_t type,num,ch;} mm_entry_t;
typedef struct {mm_entry_t map[MM_N];uint8_t color[32],ch,relative;} mm_profile_t;
typedef struct {uint32_t tag,seq;uint8_t selected,version,reserved[2];mm_profile_t profile[4];} mm_store_t;
static mm_store_t mm_data __attribute__((section(".pool")));
static mm_store_t mm_verify __attribute__((section(".pool")));
static unsigned mm_lcd_live;
static unsigned mm_slot,mm_dirty,mm_edit,mm_settings,mm_pick,mm_sel,mm_kbank,mm_fbank,mm_pbank,mm_reset_ask;
static int mm_oct;
static uint32_t mm_screen_dirty,mm_draw_t;
static const char *mm_status="READY";
static uint8_t mm_values[16],mm_toggle[MM_N],mm_fader_prev[4];
/* ABS only: speed per encoder, never shared across knobs or banks. */
static uint32_t mm_abs_time[16];
static int8_t mm_abs_dir[16];
static void mm_abs_reset(void){memset(mm_abs_time,0,sizeof mm_abs_time);memset(mm_abs_dir,0,sizeof mm_abs_dir);}
static int mm_abs_delta(unsigned i,int steps){
 steps=clampi(steps,-127,127);if(!steps)return 0;
 unsigned n=(unsigned)(steps<0?-steps:steps),gain=1;int dir=steps>0?1:-1;
 uint32_t now=fm1_ms+1u;
 if(mm_abs_time[i]&&mm_abs_dir[i]==dir){
  unsigned dt=(now-mm_abs_time[i])/n;
  gain=dt<=20u?8u:dt<=40u?4u:dt<=80u?2u:1u;
 }else if(!mm_abs_time[i]&&n>=4u)gain=4u; /* several detents collected in one panel poll */
 mm_abs_time[i]=now;mm_abs_dir[i]=(int8_t)dir;return steps*(int)gain;
}
static uint8_t mm_cv_value[MM_N];
static int16_t mm_cv_delta[16];
static uint32_t mm_cv_pending[3],mm_cv_t[MM_N];
static uint8_t mm_queue[128][3] __attribute__((section(".pool")));
static unsigned mm_w,mm_r,mm_panic_left,mm_wait_release,mm_bend_pending,mm_bend_value;
static uint16_t mm_owned[MM_SOURCES]; /* high bit valid, channel << 7 | pitch */
static uint8_t mm_release_type[MM_SOURCES];
/* ML1 runtime only: original version-1 saved maps/theme stay byte-compatible. */
static uint16_t mm_key_channels=1,mm_channel_held,mm_key_target[37],mm_key_sent[37];
static uint8_t mm_key_pitch[37],mm_key_velocity[37];
static uint64_t mm_key_retrigger;
static unsigned mm_pedal_down;
static uint16_t mm_pedal_target,mm_pedal_sent,mm_bend_channels,mm_wheel_channels[2],mm_prime_bend,mm_prime_mod;
static unsigned mm_first_channel(unsigned m){unsigned c=0;while(c<15u&&!(m>>c&1u))c++;return c;}
static unsigned mm_channel_count(void){unsigned n=0;for(unsigned c=0;c<16;c++)n+=mm_key_channels>>c&1u;return n;}
static int mm_edit_reserved(unsigned i){return (i>=MM_PAD+16u&&i<MM_BTN)||i==MM_BTN+6u||i==MM_BTN+11u||i==MM_BTN+12u;}

static uint64_t mm_keys_prev,mm_keys_blocked;
static uint32_t mm_pad_down,mm_buttons_prev,mm_buttons_blocked;
static uint8_t mm_pad_blocked[16];
static uint32_t mm_last=MM_KEYS,mm_last_v;
/* Independent display domains. Never used as MIDI ownership or saved data. */
static unsigned mm_ui_control=MM_KEYS,mm_ui_control_v,mm_ui_pad=MM_KEYS,mm_ui_pad_v,mm_ui_known,mm_ui_parts;
static uint8_t mm_ui_bar[24];
static void mm_overview_reset(void){mm_ui_control=mm_ui_pad=MM_KEYS;mm_ui_control_v=mm_ui_pad_v=mm_ui_known=0;mm_ui_parts=7;memset(mm_ui_bar,0,sizeof mm_ui_bar);}

static int mm_wheel_prev[2]={-1,-1};
/* calibration shared with the synth through mel_smk.h */
static unsigned mm_main_ch(void){return mm_data.profile[mm_data.selected&3u].ch;}
static unsigned mm_relative(void){return mm_data.profile[mm_data.selected&3u].relative;}
static mm_profile_t *mm_profile(void){return &mm_data.profile[mm_data.selected&3u];}
static int mm_reserved(unsigned i){return i==MM_BTN+6u||i==MM_BTN+11u||i==MM_BTN+12u;}
static unsigned mm_types(unsigned i){return i<MM_PAD||i==MM_MOD?((1u<<MM_OFF)|(1u<<MM_CC)):i==MM_PITCH?((1u<<MM_OFF)|(1u<<MM_CC)|(1u<<MM_BEND)):((1u<<MM_OFF)|(1u<<MM_CC)|(1u<<MM_TOGGLE)|(1u<<MM_NOTE));}
static unsigned mm_channel(const mm_entry_t *e){return (e->ch?e->ch:mm_main_ch())-1u;}
static void mm_default(mm_profile_t *p){
 memset(p,0,sizeof *p);p->ch=1;
 for(unsigned i=0;i<MM_N;i++){
  mm_entry_t *e=&p->map[i];e->type=MM_CC;
  if(i<16)e->num=(uint8_t)(i<8?70+i:102+i-8);
  else if(i<MM_PAD)e->num=(uint8_t)(20+i-16);
  else if(i<MM_BTN){unsigned n=(i-MM_PAD)%16;e->type=MM_NOTE;e->num=(uint8_t)(36+(i-MM_PAD)/16*16+(n>=8?n-8:n+8));e->ch=10;}
  else if(i<MM_PITCH)e->num=MM_BUTTON_CC[i-MM_BTN];
  else if(i==MM_PITCH)e->type=MM_BEND;
  else e->num=1;
  if(mm_reserved(i))e->type=MM_OFF;
 }
}
static int mm_valid(const mm_store_t *s){
 if(s->tag!=0x314D4D53u||s->version!=1||s->selected>3)return 0;
 for(unsigned p=0;p<4;p++){
  const mm_profile_t *v=&s->profile[p];if(v->ch<1||v->ch>16||v->relative>1)return 0;
  for(unsigned i=0;i<MM_N;i++)if(v->map[i].type>=MM_TYPES||!(mm_types(i)>>v->map[i].type&1u)||v->map[i].num>127||v->map[i].ch>16||(mm_reserved(i)&&v->map[i].type!=MM_OFF))return 0;
  for(unsigned i=0;i<32;i++)if(v->color[i]>=sizeof MM_COLOR/sizeof *MM_COLOR)return 0;
 }
 return 1;
}
static void mm_boot(void){
 memset(&mm_data,0,sizeof mm_data);mm_data.tag=0x314D4D53u;mm_data.version=1;
 for(unsigned i=0;i<4;i++)mm_default(&mm_data.profile[i]);mm_slot=1;
 for(unsigned i=0;i<2;i++)if(!mm_store_load(i,&mm_verify,sizeof mm_verify)&&mm_valid(&mm_verify)&&(!mm_data.seq||(int32_t)(mm_verify.seq-mm_data.seq)>0)){mm_data=mm_verify;mm_slot=i;}
 memset(mm_key_target,0,sizeof mm_key_target);memset(mm_key_sent,0,sizeof mm_key_sent);
 mm_prime_bend=mm_prime_mod=mm_pedal_down=mm_key_retrigger=mm_pedal_target=mm_pedal_sent=mm_bend_channels=mm_channel_held=0;
 mm_wheel_channels[0]=mm_wheel_channels[1]=0;mm_key_channels=(uint16_t)(1u<<(mm_main_ch()-1u));
 mm_abs_reset();mm_dirty=mm_on=0;mm_oct=0;mm_status="READY";mm_overview_reset();
}
static void mm_full_dirty(void){mm_ui_parts=7;mm_screen_dirty=1;}
static void mm_touch(unsigned i,unsigned v){
 mm_last=i;mm_last_v=v;mm_screen_dirty=1;
 if(i>=MM_PAD&&i<MM_BTN){mm_ui_pad=i;mm_ui_pad_v=v;mm_ui_parts|=2u;}
 else if(i<MM_PAD||(i>=MM_BTN&&i<MM_N)||i==108u){mm_ui_control=i;mm_ui_control_v=v;mm_ui_parts|=1u;}
 else mm_ui_parts|=4u;
 if(i<24u){mm_ui_known|=1u<<i;mm_ui_bar[i]=(uint8_t)clampi(v,0,127);}
}
static void mm_safety(void){
 mm_overview_reset();mm_abs_reset();
 mm_w=mm_r=0;
 /* Explicit releases precede the channel-wide reset, including devices that
  * do not implement CC123/120. Unsent onsets are discarded. */
 for(unsigned i=0;i<MM_SOURCES;i++)if((i<71u||i==108u)&&(mm_owned[i]&0x8000u)){
  unsigned wire=mm_owned[i],st=(mm_release_type[i]==MM_NOTE?0x80u:0xB0u)|(wire>>7&15u),num=wire&127u,seen=0;
  for(unsigned j=0;j<mm_w;j++)if(mm_queue[j][0]==st&&mm_queue[j][1]==num)seen=1;
  if(!seen&&mm_w<128){mm_queue[mm_w][0]=(uint8_t)st;mm_queue[mm_w][1]=(uint8_t)num;mm_queue[mm_w++][2]=0;}
 }
 for(unsigned i=MM_PAD;i<MM_PITCH;i++)if(mm_toggle[i]&&mm_w<128){mm_entry_t *e=&mm_profile()->map[i];mm_queue[mm_w][0]=(uint8_t)(0xB0u|mm_channel(e));mm_queue[mm_w][1]=e->num;mm_queue[mm_w++][2]=0;}
 /* Stream up to 592 keyboard releases without occupying the edge queue. */
 memset(mm_key_target,0,sizeof mm_key_target);mm_prime_bend=mm_prime_mod=0;mm_key_retrigger=0;mm_pedal_target=mm_pedal_down=mm_channel_held=0;
 mm_bend_channels=mm_wheel_channels[0]=mm_wheel_channels[1]=0;
 mm_panic_left=80; /* CC64/123/120, bend centre, CC1 on all channels */
 memset(mm_owned,0,sizeof mm_owned);memset(mm_release_type,0,sizeof mm_release_type);memset(mm_toggle,0,sizeof mm_toggle);
 memset(mm_cv_pending,0,sizeof mm_cv_pending);memset(mm_cv_delta,0,sizeof mm_cv_delta);
 mm_keys_prev=0;mm_keys_blocked=fm1_in.keys;mm_pad_down=0;mm_buttons_prev=fm1_in.buttons;mm_buttons_blocked=fm1_in.buttons;
 for(unsigned p=0;p<16;p++)mm_pad_blocked[p]=fm1_in.pad_st[p]!=0;
 mm_bend_pending=0;mm_wheel_prev[0]=mm_wheel_prev[1]=-1;
 for(unsigned f=0;f<4;f++)mm_fader_prev[f]=fm1_in.fader[f];
 mm_screen_dirty=1;
}
static int mm_queue_put(unsigned st,unsigned a,unsigned b){
 if(mm_panic_left)return 0;
 if(mm_w-mm_r>=128){mm_safety();mm_status="PANIC / TX OVERFLOW";return 0;}
 unsigned i=mm_w++%128;mm_queue[i][0]=(uint8_t)st;mm_queue[i][1]=(uint8_t)a;mm_queue[i][2]=(uint8_t)b;return 1;
}
static int mm_shared(unsigned source,unsigned wire){
 unsigned ch=wire>>7&15u,n=wire&127u;
 for(unsigned i=0;i<MM_SOURCES;i++)if(i!=source){
  if(i>=71u&&i<108u){unsigned k=i-71u;if((mm_key_sent[k]>>ch&1u)&&mm_key_pitch[k]==n)return 1;}
  else if(mm_release_type[i]==MM_NOTE&&mm_owned[i]==wire)return 1;
 }return 0;
}
static int mm_keyboard_release_service(void){
 for(unsigned k=0;k<37;k++){
  unsigned sent=mm_key_sent[k],want=mm_key_target[k];
  if((mm_key_retrigger>>k&1u)||!(mm_owned[71+k]&0x8000u)||mm_key_pitch[k]!=(mm_owned[71+k]&127u))want=0;
  unsigned off=sent&~want;if(!off){if(!sent)mm_key_retrigger&=~((uint64_t)1u<<k);continue;}
  unsigned ch=mm_first_channel(off),w=0x8000u|(ch<<7)|mm_key_pitch[k];mm_key_sent[k]&=(uint16_t)~(1u<<ch);
  if(!mm_key_sent[k])mm_key_retrigger&=~((uint64_t)1u<<k);
  if(!mm_shared(71+k,w))midi_send(0x80u|ch,w&127u,0);return 1;
 }return 0;
}
static int mm_keyboard_on_service(void){
 /* Group simultaneously sampled chord notes by destination channel. */
 for(unsigned ch=0;ch<16;ch++)for(unsigned k=0;k<37;k++){
  if(!(mm_owned[71+k]&0x8000u))continue;unsigned note=mm_owned[71+k]&127u;
  if(!mm_key_sent[k]){mm_key_retrigger&=~((uint64_t)1u<<k);mm_key_pitch[k]=(uint8_t)note;}
  if((mm_key_retrigger>>k&1u)||mm_key_pitch[k]!=note)continue;
  unsigned on=mm_key_target[k]&~mm_key_sent[k];if(!(on>>ch&1u))continue;
  unsigned w=0x8000u|(ch<<7)|note;if(!mm_shared(71+k,w))midi_send(0x90u|ch,note,mm_key_velocity[k]);mm_key_sent[k]|=(uint16_t)(1u<<ch);return 1;
 }return 0;
}
static int mm_pedal_service(void){
 unsigned change=mm_pedal_sent^mm_pedal_target;if(!change)return 0;
 unsigned off=mm_pedal_sent&~mm_pedal_target,ch=mm_first_channel(off?off:change),down=mm_pedal_target>>ch&1u;
 midi_send(0xB0u|ch,64,down?127:0);mm_pedal_sent^=(uint16_t)(1u<<ch);return 1;
}
static int mm_prime_service(void){
 unsigned m=mm_prime_bend|mm_prime_mod;if(!m)return 0;unsigned ch=mm_first_channel(m);
 if(mm_prime_bend>>ch&1u){mm_prime_bend&=(uint16_t)~(1u<<ch);const mm_entry_t *e=&mm_profile()->map[MM_PITCH];unsigned v=mm_wheel_prev[0]<0?8192u:(unsigned)mm_wheel_prev[0];if(e->type==MM_BEND)midi_send(0xE0u|ch,v&127u,v>>7);else if(e->type==MM_CC)midi_send(0xB0u|ch,e->num,v>>7);}
 else{mm_prime_mod&=(uint16_t)~(1u<<ch);const mm_entry_t *e=&mm_profile()->map[MM_MOD];if(e->type==MM_CC)midi_send(0xB0u|ch,e->num,mm_wheel_prev[1]<0?0u:(unsigned)mm_wheel_prev[1]);}return 1;
}
static void mm_release(unsigned source){
 if(source>=71u&&source<108u){unsigned k=source-71u;mm_key_target[k]=0;mm_key_retrigger|=(uint64_t)1u<<k;mm_owned[source]=mm_release_type[source]=0;return;}

 if(source>=MM_PAD&&source<MM_PITCH&&mm_profile()->map[source].type==MM_CC){
  if(source==mm_ui_pad){mm_ui_pad_v=0;mm_ui_parts|=2u;mm_screen_dirty=1;}
  if(source==mm_ui_control){mm_ui_control_v=0;mm_ui_parts|=1u;mm_screen_dirty=1;}
 }
 if(source==mm_last&&source>=MM_PAD&&source<MM_PITCH&&mm_profile()->map[source].type==MM_CC)mm_touch(source,0);
 unsigned w=mm_owned[source],t=mm_release_type[source];mm_owned[source]=mm_release_type[source]=0;
 if(w&0x8000u){unsigned ch=w>>7&15u,n=w&127u;if(t==MM_NOTE){if(!mm_shared(source,w))mm_queue_put(0x80u|ch,n,0);}else if(t==MM_CC)mm_queue_put(0xB0u|ch,n,0);}
}
static void mm_press(unsigned source,const mm_entry_t *e,unsigned vel){
 /* Pad feedback is physical, independent of a blocked output safety flush. */
 mm_touch(source,e->type==MM_OFF?0u:e->type==MM_CC?127u:vel);
 if(mm_panic_left||e->type==MM_OFF)return;
 mm_release(source);mm_touch(source,e->type==MM_CC?127u:vel);
 if(source>=71u&&source<108u){unsigned k=source-71u;mm_owned[source]=(uint16_t)(0x8000u|e->num);mm_release_type[source]=MM_NOTE;mm_key_target[k]=mm_key_channels;mm_key_velocity[k]=(uint8_t)vel;return;}
 unsigned ch=mm_channel(e),w=0x8000u|(ch<<7)|e->num;
 if(e->type==MM_NOTE){if(mm_shared(source,w)||mm_queue_put(0x90u|ch,e->num,vel)){mm_owned[source]=(uint16_t)w;mm_release_type[source]=MM_NOTE;}}
 else if(e->type==MM_CC){if(mm_queue_put(0xB0u|ch,e->num,127)){mm_owned[source]=(uint16_t)w;mm_release_type[source]=MM_CC;}}
 else if(e->type==MM_TOGGLE){unsigned v=mm_toggle[source]?0:127;if(mm_queue_put(0xB0u|ch,e->num,v))mm_toggle[source]=(uint8_t)!!v;}
}
static void mm_cv(unsigned i,int v,int relative){
 /* Physical feedback and coalescing must not wait for a host to read MIDI.
  * mm_service still sends the complete safety flush before these values. */
 if(mm_profile()->map[i].type==MM_OFF){mm_touch(i,0);return;}
 if(relative){int d=mm_cv_delta[i]+v;mm_cv_delta[i]=(int16_t)clampi(d,-8192,8192);}
 else mm_cv_value[i]=(uint8_t)clampi(v,0,127);
 if(i>=MM_PITCH)mm_wheel_channels[i-MM_PITCH]=mm_key_channels;
 mm_cv_pending[i>>5]|=1u<<(i&31);mm_touch(i,relative?(unsigned)(64+clampi(v,-63,63)):(unsigned)v);
}
static void mm_safety_complete(void);
static void mm_service(void){
 unsigned safety=mm_panic_left;
 for(unsigned n=0;n<12&&midi_send_ready();n++){
  if(mm_pedal_service())continue;
  if(mm_keyboard_release_service())continue;
  if(mm_panic_left&&mm_r!=mm_w){unsigned i=mm_r++%128;midi_send(mm_queue[i][0],mm_queue[i][1],0);continue;}
  if(mm_panic_left){unsigned i=80-mm_panic_left,ch=i/5,t=i%5;static const uint8_t cc[]={64,123,120,1};if(t==3)midi_send(0xE0u|ch,0,64);else midi_send(0xB0u|ch,cc[t==4?3:t],0);mm_panic_left--;continue;}
  if(mm_r!=mm_w){unsigned i=mm_r++%128;midi_send(mm_queue[i][0],mm_queue[i][1],mm_queue[i][2]);continue;}
  if(!mm_on||mm_edit||mm_settings)break;
  if(mm_prime_service())continue;
  if(mm_keyboard_on_service())continue;
  if(mm_bend_pending){if(!mm_bend_channels)mm_bend_channels=mm_key_channels;unsigned ch=mm_first_channel(mm_bend_channels);midi_send(0xE0u|ch,mm_bend_value&127u,mm_bend_value>>7);mm_bend_channels&=(uint16_t)~(1u<<ch);if(!mm_bend_channels)mm_bend_pending=0;continue;}
  unsigned i;for(i=0;i<MM_N;i++)if((mm_cv_pending[i>>5]>>(i&31)&1u)&&fm1_ms-mm_cv_t[i]>=8)break;
  if(i==MM_N)break;
  mm_entry_t e=mm_profile()->map[i];unsigned v=mm_cv_value[i];int rel=i<16&&mm_relative();
  if(rel){int d=clampi(mm_cv_delta[i],-63,63);v=(unsigned)(64+d);mm_cv_delta[i]-=(int16_t)d;}
  if(i>=MM_PITCH){unsigned w=i-MM_PITCH;if(!mm_wheel_channels[w])mm_wheel_channels[w]=mm_key_channels;unsigned ch=mm_first_channel(mm_wheel_channels[w]);midi_send(0xB0u|ch,e.num,v);mm_wheel_channels[w]&=(uint16_t)~(1u<<ch);if(mm_wheel_channels[w])continue;}
  else midi_send(0xB0u|mm_channel(&e),e.num,v);mm_cv_t[i]=fm1_ms;
  if(!rel||!mm_cv_delta[i])mm_cv_pending[i>>5]&=~(1u<<(i&31));
 }
 if(safety&&!mm_panic_left)mm_safety_complete();
}
static void mm_enter(void);
static void mm_leave(void);
static void mm_setting_turn(unsigned k,int e);

static void mm_pedal_event(unsigned down);

static void mm_channels_press(unsigned p){
 unsigned previous=mm_key_channels,bit=1u<<(p&15u);mm_key_channels=(uint16_t)(mm_channel_held?(mm_key_channels|bit):bit);mm_channel_held|=(uint16_t)bit;
 if(mm_pedal_down)mm_pedal_target|=mm_key_channels;
 unsigned added=mm_key_channels&~previous;
 if(mm_profile()->map[MM_PITCH].type!=MM_OFF)mm_prime_bend|=(uint16_t)added;
 if(mm_profile()->map[MM_MOD].type!=MM_OFF)mm_prime_mod|=(uint16_t)added;
 if(mm_bend_pending)mm_bend_channels=mm_key_channels;
 for(unsigned w=0;w<2;w++)if(mm_cv_pending[(MM_PITCH+w)>>5]>>((MM_PITCH+w)&31)&1u)mm_wheel_channels[w]=mm_key_channels;
 mm_status="READY";mm_full_dirty();
}
