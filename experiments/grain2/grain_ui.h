/* SPDX-License-Identifier: GPL-3.0-only. Persistent GRAIN effect; modulation wheel controls vibrato. */
static const char *const GR_NAMES[]={"SIZE ms","SPREAD %","DENSITY %","MIX %"};
static uint32_t gr_focus,gr_return;
static __attribute__((noinline)) void gr_store(uint32_t k){set.p[P_AGATE]=(int16_t)(gr_ctl[3]|(gr_on<<7)|(gr_style<<8));changed(k);redraw=0xFFu;}
static void gr_toggle(void){gr_on=!gr_on;gr_store(99);say(gr_on?"GRAIN ON":"GRAIN OFF");}
static void gr_style_select(int32_t d){
 static const uint16_t values[3][4]={{200,65,100,75},{70,25,35,85},{35,100,85,100}};
 uint32_t k;gr_style=(uint32_t)(((int32_t)gr_style+d)%4+4)%4;
 if(gr_style){for(k=0;k<4;k++)gr_ctl[k]=values[gr_style-1u][k];}
 set.p[P_AMODE]=(int16_t)gr_ctl[0];set.p[P_ARATE]=(int16_t)gr_ctl[1];set.p[P_AOCT]=(int16_t)gr_ctl[2];gr_store(99);
}
static void gr_knob(uint32_t k,int32_t d){
 static const int16_t lo[]={20,0,15,0},hi[]={400,100,100,100};
 int32_t v=(int32_t)gr_ctl[k]+d;if(v<lo[k])v=lo[k];if(v>hi[k])v=hi[k];
 if(v==(int32_t)gr_ctl[k])return;gr_ctl[k]=(uint32_t)v;gr_style=0;
 if(k<3u)set.p[P_AMODE+k]=(int16_t)v;gr_store(k);
}
static void gr_wheel_focus(void){
 static int32_t last;
 if(gr_focus&&page!=PG_VIB)gr_focus=0; /* explicit navigation wins */
 if(p_mod==last)return;last=p_mod;
 if(p_mod){if(!gr_focus){gr_return=page;gr_focus=1;}page=PG_VIB;touched=99;}
 else if(gr_focus){page=gr_return;gr_focus=0;}
 redraw=0xFFu;
}
static void gr_draw(uint32_t base){
 if(smktron_zoom){zm_draw(base);return;}
 uint32_t k,v;char s[28];uint16_t accent=SP_ACCENT[PV(P_SOUND)];
 static uint32_t last;uint32_t sig=gr_ctl[0]+gr_ctl[1]*401u+gr_ctl[2]*40501u+gr_ctl[3]*4090501u;
 sig=sig*33u+gr_on+gr_style*2u+touched*11u+by_fx*64u;sig=sig*33u+PV(P_SOUND);for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp)sig=sig*33u+voice[v].note;
 if(sig==last&&!redraw)return;last=sig;
 sp_header_title(PV(P_SOUND),"GRAIN");
 cv_begin(240,42,SP_BG);cv_oy=-24;
 sp_text(12,28,&SP_F17,"GRAIN",SP_INK,SP_BG);sp_right(228,35,&SP_F8,gr_on?(by_fx?"BYPASS":"ON"):"OFF",accent,SP_BG);
 put(put(s,"K6 STYLE: "),GR_STYLE[gr_style]);sp_text(12,50,&SP_F8,s,accent,SP_BG);cv_oy=0;cv_blit(0,24);
 {
  cv_begin(240,64,SP_BG);cv_oy=-66;
  /* Static overlapping tape fragments; screen work never touches audio heads. */
  for(k=0;k<7u;k++){int32_t x=16+(int32_t)k*29,y=79+(int32_t)(k%3u)*7;rrect(x,y,27,29,3,SP_LINE);cv_rect(x+4,y+7,19,2,accent);cv_rect(x+7,y+14,13,2,SP_MUT);}
  sp_text(12,117,&SP_F8,MEL_SET_NAME[PV(P_SOUND)],accent,SP_BG);cv_oy=0;cv_blit(0,66);
  cv_begin(240,36,SP_BG);cv_oy=-130;sp_text(12,139,&SP_F8,gr_on?"PAD 10: ON":"PAD 10: OFF",SP_MUT,SP_BG);cv_oy=0;cv_blit(0,130);
  sp_keyboard(base,1);
  cv_begin(240,40,SP_PANEL);cv_oy=-200;
  for(k=0;k<4u;k++){int32_t x=(int32_t)k*60;sp_text(x+5,202,&SP_F7,GR_NAMES[k],SP_INK,SP_PANEL);put_int(s,(int32_t)gr_ctl[k],0);tx(x+5,217,&F16,s,SP_INK);}
  cv_oy=0;cv_blit(0,200);return;
 }
}
