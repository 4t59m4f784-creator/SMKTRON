/* SPDX-License-Identifier: GPL-3.0-only */
/* Included after the synth's input/panic handlers. */
static void mm_pedal_event(unsigned down){
 if(mm_edit||mm_settings||mm_panic_left)return;
 mm_entry_t e={MM_CC,64,0};
 if(down)mm_press(108,&e,127);else mm_release(108);
}
static void mm_setting_turn(unsigned k,int e){
 if(!e)return;
 if(!k){if(e>0&&!mm_on)mm_enter();else if(e<0&&mm_on)mm_leave();return;}
 if(mm_on)mm_safety();
 if(k==1)mm_data.selected=(uint8_t)clampi((int)mm_data.selected+(e>0?1:-1),0,3);
 else if(k==2)mm_profile()->ch=(uint8_t)clampi((int)mm_main_ch()+e,1,16);
 else mm_profile()->relative=e>0;
 memset(mm_values,0,sizeof mm_values);mm_dirty=1;mm_screen_dirty=1;
}
static void mm_save(void){
 if(fm1_in.keys||mm_pad_down||mm_pedal.down){mm_status="RELEASE KEYS / PEDAL";mm_screen_dirty=1;return;}
 /* Saving an edited map never writes the synth's settings. Alternate CRC
  * sectors; verify before selecting the new generation. Last good survives. */
 mm_safety();unsigned old=mm_data.seq;mm_data.seq++;if(!mm_data.seq)mm_data.seq=1;
 unsigned slot=mm_slot^1u;
 if(mm_store_save(slot,&mm_data,sizeof mm_data)||mm_store_load(slot,&mm_verify,sizeof mm_verify)||memcmp(&mm_verify,&mm_data,sizeof mm_data)){
  mm_data.seq=old;mm_status="SAVE FAILED / RETRY";
 }else{mm_slot=slot;mm_dirty=0;mm_status="SAVED";}
 mm_screen_dirty=1;
}
static void mm_name(unsigned i,char *b){
 b[0]=0;
 if(i<16){scat(b,"KNOB ");scat(b,i<8?"A":"B");fmt_u(b+slen(b),i%8+1);}
 else if(i<MM_PAD){scat(b,"FADER ");scat(b,i<20?"A":"B");fmt_u(b+slen(b),i%4+1);}
 else if(i<MM_BTN){scat(b,"PAD ");scat(b,i<40?"A":"B");fmt_u(b+slen(b),(i-MM_PAD)%16+1);}
 else if(i<MM_PITCH)scat(b,MM_BUTTON_NAME[i-MM_BTN]);
 else scat(b,i==MM_PITCH?"PITCH WHEEL":i==MM_MOD?"MOD WHEEL":"KEYBOARD");
}
static unsigned mm_pad_colour(unsigned i){unsigned c=mm_profile()->color[i-MM_PAD];if(c)return c;unsigned t=mm_profile()->map[i].type;return t==MM_NOTE?(i<40?6:4):t==MM_OFF?9:3;}
static void mm_leds(void){
 for(unsigned b=0;b<18;b++)fm1_led_key(b,0);
 fm1_led_key(BTN_KNOB_BANK,mm_kbank);fm1_led_key(BTN_FADER_BANK,mm_fbank);
 fm1_led_key(BTN_GLO,mm_settings);fm1_led_key(BTN_EDIT,mm_edit);fm1_led_key(BTN_SAVE,mm_dirty&&((fm1_ms/350u)&1u));
 fm1_led_key(BTN_OCT_DOWN,mm_oct<0);fm1_led_key(BTN_OCT_UP,mm_oct>0);
 for(unsigned b=0;b<13;b++)if(!mm_reserved(MM_BTN+b))fm1_led_key(MM_BUTTON[b],mm_toggle[MM_BTN+b]||((mm_buttons_prev>>MM_BUTTON[b])&1u));
 for(unsigned p=0;p<16;p++){
  unsigned i=MM_PAD+16*mm_pbank+p,c=mm_pad_colour(i),active=mm_toggle[i]||(mm_pad_down>>p&1u),level=active?255:80;
  if(mm_profile()->map[i].type==MM_OFF)level=12;
  if(mm_edit&&!mm_pick&&mm_sel==i){c=9;level=(fm1_ms/200u&1u)?230:45;}
  padled_set(p,(uint8_t)(MM_COLOR[c][0]*level*smktron_pad_scale/1020u),(uint8_t)(MM_COLOR[c][1]*level*smktron_pad_scale/1020u),(uint8_t)(MM_COLOR[c][2]*level*smktron_pad_scale/1020u));
 }
 padled_flush();
}
static void mm_select(unsigned i){if(mm_reserved(i))return;mm_sel=i;mm_pick=0;mm_screen_dirty=1;}
static void mm_edit_turn(unsigned k,int e){
 if(!e||mm_reserved(mm_sel))return;
 mm_entry_t *v=&mm_profile()->map[mm_sel];
 if(!k){int t=v->type;do{t=(t+(e>0?1:MM_TYPES-1))%MM_TYPES;}while(!(mm_types(mm_sel)>>t&1u));v->type=(uint8_t)t;}
 else if(k==1)v->num=(uint8_t)clampi((int)v->num+e,0,127);
 else if(k==2)v->ch=(uint8_t)clampi((int)v->ch+e,0,16);
 else if(mm_sel>=MM_PAD&&mm_sel<MM_BTN){uint8_t *c=&mm_profile()->color[mm_sel-MM_PAD];*c=(uint8_t)clampi((int)*c+(e>0?1:-1),0,9);}
 mm_dirty=1;mm_screen_dirty=1;mm_status="EDIT / BT SAVE";
}
/* Shared by the normal panel poll and LCD yields. No navigation, flash or drawing. */
static void mm_keys_poll(void){
 /* Timer ISR publishes a 64-bit mask on a 32-bit CPU: take one coherent snapshot. */
 fm1_irq_off();uint64_t keys=fm1_in.keys;fm1_irq_on();mm_keys_blocked&=keys;uint64_t down=keys&~mm_keys_prev&~mm_keys_blocked,up=mm_keys_prev&~keys;mm_keys_prev=keys;
 for(unsigned k=0;k<37;k++){
  if(up>>k&1u)mm_release(71+k);
  if((down>>k&1u)&&!mm_edit&&!mm_settings&&!mm_reset_ask){mm_entry_t e={MM_NOTE,(uint8_t)clampi(48+(int)k+12*mm_oct,0,127),0};mm_press(71+k,&e,fm1_in.key_vel[k]?fm1_in.key_vel[k]:100);mm_touch(MM_KEYS,e.num);}
 }
 while(smk_extkey_take()>=0){} /* keys are read once as the complete 37-bit mask */
}
static void mm_lcd_idle(void){
 static unsigned busy,seen;
 static uint32_t last_ms;
 if(!mm_on||!mm_lcd_live||mm_exiting||busy)return;
 /* At most 1 ms between input opportunities; a pending queue can drain
  * between those opportunities. This is main-context cooperation, not an ISR. */
 unsigned now=fm1_ms;
 if(seen&&now==last_ms&&mm_r==mm_w&&!mm_panic_left)return;
 busy=1;
 if(!seen||now!=last_ms){seen=1;last_ms=now;mm_keys_poll();mm_sustain_poll();}
 mm_service();
 busy=0;
}
#include "midi_draw.h"
static void mm_input(void){
 uint32_t rel,b=fm1_input_edges(&rel),cur=fm1_in.buttons;
 const unsigned pair=(1u<<BTN_GLO)|(1u<<BTN_PLAY);
 static unsigned panic_pair;
 if((cur&pair)==pair&&!panic_pair){mm_safety();mm_pedal.blocked=1;mm_pedal.down=0;mm_status="MIDI PANIC";panic_pair=1;}
 if(panic_pair){b&=~pair;rel&=~pair;if(!(cur&pair))panic_pair=0;}
 mm_buttons_blocked&=cur;b&=~mm_buttons_blocked;rel&=~mm_buttons_blocked;
 if(mm_reset_ask){
  if(b>>BTN_HOME&1u){mm_reset_ask=0;mm_screen_dirty=1;}
  if(b>>BTN_SAVE&1u){mm_default(mm_profile());mm_dirty=1;mm_reset_ask=0;mm_save();}
 }else{
  if(rel>>BTN_GLO&1u){mm_safety();mm_settings=!mm_settings;mm_edit=mm_pick=0;mm_status=mm_settings?"K1 LEFT: SYNTH":"READY";}
  if(b>>BTN_EDIT&1u){mm_safety();mm_edit=!mm_edit;mm_settings=0;mm_pick=mm_edit;mm_status=mm_edit?"PARA EXIT / BT SAVE":"READY";}
  if(b>>BTN_SAVE&1u)mm_save();
  if(b>>BTN_HOME&1u){if(mm_edit){mm_pick=1;mm_screen_dirty=1;}else if(mm_settings){mm_settings=0;mm_screen_dirty=1;}}
  if(mm_settings&&(b>>BTN_REC&1u)){mm_reset_ask=1;mm_screen_dirty=1;}
 }
 if((b&3u)==3u||(cur&3u)==3u){if(b&3u){mm_pbank^=1;mm_screen_dirty=1;}b&=~3u;}
 else {if(b>>BTN_KNOB_BANK&1u){mm_kbank^=1;mm_abs_reset();mm_screen_dirty=1;}if(b>>BTN_FADER_BANK&1u){mm_fbank^=1;mm_screen_dirty=1;}}
 if(b&((1u<<BTN_OCT_DOWN)|(1u<<BTN_OCT_UP))){
  if(mm_edit){int i=(int)mm_sel;do{i=(i+(b>>BTN_OCT_UP&1u?1:MM_N-1))%MM_N;}while(mm_reserved((unsigned)i));mm_select((unsigned)i);}
  else {mm_oct=clampi(mm_oct+(b>>BTN_OCT_UP&1u?1:-1),-3,3);mm_screen_dirty=1;}
 }
 mm_keys_poll();
 int ev;while((ev=smk_pad_take())>=0){unsigned p=((unsigned)ev>>8)&15u,vel=(unsigned)ev&127u,i=MM_PAD+16*mm_pbank+p;
  if(!vel){mm_pad_blocked[p]=0;mm_pad_down&=~(1u<<p);mm_release(MM_PAD+p);mm_release(MM_PAD+16+p);mm_screen_dirty=1;continue;}
  if(mm_edit){mm_select(i);continue;}
  if(mm_pad_blocked[p]||mm_settings||mm_reset_ask)continue;
  if(!(mm_pad_down>>p&1u)){mm_pad_down|=1u<<p;mm_press(i,&mm_profile()->map[i],vel);}
 }
 for(unsigned k=0;k<8;k++){int e=fm1_enc_take(k);if(!e)continue;unsigned i=8*mm_kbank+k;
  if(mm_reset_ask)continue;
  if(mm_settings){if(k<4)mm_setting_turn(k,e);if(!mm_on||mm_exiting)return;}
  else if(mm_edit){if(mm_pick){mm_select(i);}else if(k<4)mm_edit_turn(k,e);else if(k==4){int n=(int)mm_sel;unsigned steps=(unsigned)(e<0?-e:e);if(steps>MM_N)steps=MM_N;while(steps--){do{n=(n+(e>0?1:MM_N-1))%MM_N;}while(mm_reserved((unsigned)n));}mm_select((unsigned)n);}}
  else {if(mm_relative())mm_cv(i,e,1);else{mm_values[i]=(uint8_t)clampi((int)mm_values[i]+mm_abs_delta(i,e),0,127);mm_cv(i,mm_values[i],0);}}
 }
 for(unsigned f=0;f<4;f++){unsigned v=fm1_in.fader[f],i=16+4*mm_fbank+f;if(v==mm_fader_prev[f])continue;mm_fader_prev[f]=(uint8_t)v;if(mm_edit){if(mm_pick)mm_select(i);}else if(!mm_settings&&!mm_reset_ask)mm_cv(i,(int)v,0);}
 for(unsigned j=0;j<13;j++){unsigned i=MM_BTN+j,bit=1u<<MM_BUTTON[j];if(mm_reserved(i))continue;
  if(rel&bit)mm_release(i);
  if(b&bit){if(mm_edit){if(mm_pick)mm_select(i);}else if(!mm_settings&&!mm_reset_ask)mm_press(i,&mm_profile()->map[i],127);}
 }
 mm_buttons_prev=cur;
 mm_sustain_poll();
}
static void mm_wheels_poll(void){
 if(fm1_in.frames<300u||mm_panic_left)return;
 int bend=clampi(smk_pw_bend(&smk_pitch_calibration,fm1_in.wheel[0],fm1_ms)+8192,0,16383);
 static int lo=485,hi=615;int raw=fm1_in.wheel[1];if(raw<lo)lo=raw;if(raw>hi)hi=raw;
 int mod=clampi((hi-raw-12)*127/(hi-lo-12>1?hi-lo-12:1),0,127);
 int value[2]={bend,mod};
 for(unsigned w=0;w<2;w++){
  if(mm_edit&&mm_wheel_prev[w]<0){mm_wheel_prev[w]=value[w];continue;}
  if(value[w]==mm_wheel_prev[w])continue;mm_wheel_prev[w]=value[w];unsigned i=MM_PITCH+w;
  if(mm_edit){if(mm_pick)mm_select(i);continue;}
  if(mm_settings||mm_reset_ask||mm_panic_left)continue;
  mm_entry_t *e=&mm_profile()->map[i];
  if(e->type==MM_BEND){/* coalesced separately from 7-bit CC values */mm_bend_value=(unsigned)value[w];mm_bend_pending=1;mm_touch(i,(unsigned)value[w]);}
  else if(e->type==MM_CC)mm_cv(i,w?value[w]:value[w]>>7,0);
 }
}
static void mm_poll(void){
 if(mm_exiting){mm_service();mm_discard_midi();if(!mm_panic_left&&mm_w==mm_r&&midi_wire_idle())mm_finish_leave();return;}
 mm_input();if(!mm_on||mm_exiting)return;
 mm_wheels_poll();mm_service();mm_discard_midi();mm_leds();mm_draw();
}
