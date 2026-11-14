/* SPDX-License-Identifier: GPL-3.0-only
 * GR3 presentation only. No audio RNG, DSP state, persistence or timer writes.
 * At most 12.5 small screen updates/s. LED updates use the existing UI cadence.
 */
static uint32_t gr_visual_live(void){
 uint32_t v;if(!gr_enabled||!gr_ctl[3])return 0;
 for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp)return 1;
 return 0;
}
static uint32_t gr_visual_period(void){
 uint32_t p=gr_ctl[0]*600u/gr_ctl[2];return p<480u?480u:p>2000u?2000u:p;
}
static uint32_t gr_visual_phase(void){return (fm1_ms%gr_visual_period())*256u/gr_visual_period();}
static void gr_fragment_stage(void){
 uint32_t k,count=3u+gr_ctl[2]*6u/100u,live=gr_visual_live(),phase=live?gr_visual_phase():128u;
 uint16_t accent=gr_enabled?SP_ACCENT[PV(P_SOUND)]:SP_MUT;
 int32_t w=10+(int32_t)(gr_ctl[0]*28u/400u),range=2+(int32_t)gr_ctl[1]/10;
 cv_begin(240,64,SP_BG);cv_oy=-66;
 cv_line(12,95,228,95,SP_LINE);cv_line(12,111,228,111,SP_LINE);
 for(k=0;k<count;k++){
  uint32_t seed=k*73u+17u,p=(phase+k*256u/count)&255u;
  int32_t x=12+(int32_t)(p*192u/256u),y=86+((int32_t)(seed%33u)-16)*range/16;
  uint32_t fade=p<128u?p:255u-p;uint16_t fill=fade>36u&&gr_enabled?SP_PANEL:SP_BG;
  rrect(x,y,w,17,3,fade>30u?accent:SP_LINE);rrect(x+1,y+1,w-2,15,2,fill);
  cv_rect(x+3,y+4,w-6,2,fade>30u?accent:SP_LINE);cv_rect(x+4,y+10,w-8,1,SP_MUT);
  if(live&&fade>80u)cv_rect(x+w/2,77-(int32_t)(seed%6u),2,2,accent);
 }
 cv_line(120,73,120,113,live?RGB(255,144,32):SP_MUT);cv_rect(118,71,5,2,accent);
 sp_text(12,117,&SP_F8,MEL_SET_NAME[PV(P_SOUND)],accent,SP_BG);
 cv_oy=0;cv_blit(0,66);
}
static void gr_zoom_strip(uint32_t force){
 static uint32_t last,was=99;uint32_t live=gr_visual_live();
 if(!force&&live==was&&(!live||fm1_ms-last<80u))return;last=fm1_ms;was=live;
 uint32_t k,count=3u+gr_ctl[2]*6u/100u,phase=live?gr_visual_phase():128u;
 uint16_t accent=gr_enabled?SP_ACCENT[PV(P_SOUND)]:SP_MUT;
 cv_begin(240,12,SP_BG);cv_oy=-210;cv_line(8,216,231,216,SP_LINE);
 for(k=0;k<count;k++){
  uint32_t p=(phase+k*256u/count)&255u;int32_t x=8+(int32_t)(p*204u/256u),w=4+(int32_t)gr_ctl[0]/40;
  cv_rect(x,212,w,8,accent);cv_rect(x+1,214,w-2,4,SP_PANEL);
 }
 cv_oy=0;cv_blit(0,210);
}
static void gr_pad_visual(uint32_t tick){
 if(!tick)return;
 if(!gr_on){padled_set(9,16,4,2);return;}
 if(!gr_enabled||!gr_ctl[3]){padled_set(9,64,18,9);return;}
 if(!gr_visual_live()){padled_set(9,144,48,24);return;}
 uint32_t p=gr_visual_phase(),t=p<128u?p:255u-p,level=112u+t*143u/127u;
 /* Coral to amber, a rounded pulse; intensity follows the grain cadence. */
 padled_set(9,level,level*(48u+t*80u/127u)/255u,level*(76u-t*48u/127u)/255u);
}
