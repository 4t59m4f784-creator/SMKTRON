/* SPDX-License-Identifier: GPL-3.0-only
 * Eight musical snapshots. Appearance is global and excluded from scenes.
 * Short SAVE writes boot on release; long SAVE never writes boot.
 * Scene pad short release recalls; 1 s hold saves once without recalling. */
static void sc_notice(const char *action,uint32_t p){char s[32];put_int(put(s,action),(int32_t)p+1,0);say(s);}
static void sc_mode(uint32_t on){sc_on=on;sc_ignore=sc_held;sc_fired=0;if(!on)t_msg=0;redraw=0xFFu;}
static void sc_capture(mel_set_t *dst){
 set.master=(uint16_t)smk_master;set.bypass=(uint8_t)((set.bypass&56u)|by_lofi|(by_fx<<1)|(smk_bank<<2));
 memcpy(dst,&set,sizeof set);dst->bypass&=7u; /* appearance is global, never captured */
}
static int sc_valid(const mel_set_t *s){
 uint32_t i;if(s->ver!=SET_VER||s->np!=NP||s->master>1023u||s->half>1u||(s->bypass&~63u))return 0;
 for(i=0;i<NP;i++)if(s->p[i]<PINFO[i][0]||s->p[i]>PINFO[i][1])return 0;
 for(i=0;i<64u;i++)if(s->steps[i/4u][i%4u]>127u)return 0;
 return 1;
}
static void sc_recall(uint32_t p){
 const mel_set_t *s=&sc_bank.slot[p];uint32_t i;
 if(!(sc_bank.mask&(1u<<p))){sc_notice("EMPTY ",p);return;}
 if(!sc_valid(s)){sc_notice("INVALID ",p);return;}
 seq_start(0);seq_rec=0;arp_release();chord_open=0;seq_cur=0;
 memcpy(&set,s,sizeof set);set.bypass=(uint8_t)((set.bypass&7u)|((4u-smktron_pad_scale)<<3)|(smktron_theme<<5));by_lofi=set.bypass&1u;by_fx=(set.bypass>>1)&1u;smk_bank=(set.bypass>>2)&1u;smk_master=set.master;
#if !defined(DINGHY_WEB) && !defined(DINGHY_HOST)
 knob_avg=smk_master*16;master_q12=(smk_master*smk_master)>>8;
#endif
 for(i=0;i<4u;i++){smk_fad_last[i]=0xFF;smk_fxmem[i]=PV(SMK_FXP[i])?(uint8_t)PV(SMK_FXP[i]):SMK_FXMIX0[i];}
 apply();dirty=1;t_change=fm1_ms;touched=99;sc_selected=p;
 page=PV(P_BSND)?PG_LAYER:PG_PLAY;g_last[G_PLAY]=(uint8_t)page;redraw=0xFFu;
 sc_notice("SCENE ",p);
}
static void sc_save(uint32_t p){
 mel_set_t old=sc_bank.slot[p];uint32_t mask=sc_bank.mask;
 sc_capture(&sc_bank.slot[p]);sc_bank.mask|=1u<<p;
 if(sc_store_save(&sc_bank,sizeof sc_bank)){sc_bank.slot[p]=old;sc_bank.mask=mask;say("SCENE SAVE FAILED");return;}
 sc_selected=p;sc_ack_pad=p;sc_ack_ms=fm1_ms;sc_notice("SCENE SAVED ",p);
}
static int sc_pad_event(uint32_t p,uint32_t vel){
 uint32_t bit=1u<<p,was=sc_held&bit;
 if(vel)sc_held|=bit;else sc_held&=~bit;
 if(!sc_on)return 0;
 if(sc_ignore&bit){if(!vel)sc_ignore&=~bit;return 1;}
 if(p<8u){
  if(vel&&!was){sc_pad_ms[p]=fm1_ms;sc_fired&=~bit;}
  if(!vel&&was&&!(sc_fired&bit)){
   if(fm1_ms-sc_pad_ms[p]>=1000u)sc_save(p);else sc_recall(p);
  }
 }else if(p==8u&&vel&&!was){sc_pin=!sc_pin;if(!sc_pin&&!(fm1_in.buttons&(1u<<BTN_SAVE)))sc_mode(0);say(sc_pin?"PRESETS FIXED":"PRESETS RELEASED");}
 return 1;
}
static void sc_buttons(uint32_t b,uint32_t rel){
 uint32_t i;
 if(b&(1u<<BTN_SAVE)){sc_save_ms=fm1_ms;sc_save_down=1;sc_save_long=0;}
 if(sc_save_down&&(fm1_in.buttons&(1u<<BTN_SAVE))&&!sc_save_long&&fm1_ms-sc_save_ms>=600u){sc_save_long=1;if(!sc_on)sc_mode(1);say(sc_pin?"PRESETS FIXED":"SCENES");}
 if((rel&(1u<<BTN_SAVE))&&sc_save_down){
  sc_save_down=0;
  if(!sc_save_long){save_req=1;say("SAVING BOOT");}
  else if(!sc_pin)sc_mode(0);
 }
 if(sc_on)for(i=0;i<8u;i++)if((sc_held&~sc_ignore&~sc_fired)&(1u<<i)){
  if(fm1_ms-sc_pad_ms[i]>=1000u){sc_fired|=1u<<i;sc_save(i);}
 }
}
static void sc_leds(void){
 static uint32_t last;uint32_t i;if(fm1_ms-last<40u)return;last=fm1_ms;
 for(i=0;i<16u;i++)padled_set(i,0,0,0);
 for(i=0;i<8u;i++){
  uint32_t filled=sc_bank.mask&(1u<<i),v=filled?160u:24u;
  if(i==sc_selected&&filled)v=255;
  if((sc_held&~sc_ignore&~sc_fired)&(1u<<i)){uint32_t dt=fm1_ms-sc_pad_ms[i];if(dt>1000u)dt=1000u;v=48u+dt*207u/1000u;}
  if(i==sc_ack_pad&&fm1_ms-sc_ack_ms<450u)v=((fm1_ms-sc_ack_ms)/90u)&1u?80u:255u;
  padled_set(i,(uint8_t)(v/3u),(uint8_t)v,(uint8_t)(v*3u/4u));
 }
 padled_set(8,sc_pin?255:48,sc_pin?224:40,sc_pin?8:2);
}
