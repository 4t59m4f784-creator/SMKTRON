/* SPDX-License-Identifier: GPL-3.0-only
 * KNOB BANK short release cycles the live pad owners. SAVE/GLOBE remain valid. */
static uint32_t pm_down,pm_time,pm_return;
static void pm_buttons(uint32_t b,uint32_t rel){
 if(b&(1u<<BTN_KNOB_BANK)){pm_down=1;pm_time=fm1_ms;}
 if((rel&(1u<<BTN_KNOB_BANK))&&pm_down){
  pm_down=0;
  if(fm1_ms-pm_time>=600u)return;
  if(ch_on){ch_mode(0);page=pm_return;}
  else if(sc_on){sc_pin=0;sc_mode(0);ch_mode(1);}
  else {if(page!=PG_CHORD)pm_return=page;sc_mode(1);sc_pin=1;}
  redraw=255u;
 }
}
static void pm_draw(uint32_t base){
 static uint32_t last;uint32_t i;char text[24];
 if(!redraw&&fm1_ms-last<100u)return;last=fm1_ms;
 sp_header_title((uint32_t)PV(P_SOUND),"PRESETS");
 cv_begin(240,124,SP_BG);
 sp_text(15,7,&SP_F10,"FAVORITES",SP_INK,SP_BG);
 for(i=0;i<8u;i++){
  int32_t x=12+(int32_t)(i%2u)*114,y=28+(int32_t)(i/2u)*23;
  uint16_t color=i==sc_loaded?SP_GREEN:SP_PANEL;
  rrect(x,y,106,20,2,color);put_int(text,(int32_t)i+1u,0);
  sp_text(x+4,y+4,&SP_F10,text,SP_INK,color);
  sp_text(x+22,y+6,&SP_F7,sc_bank.mask&(1u<<i)?MEL_SET_NAME[(uint32_t)sc_bank.slot[i].p[P_SOUND]%MEL_NSETS]:"EMPTY",SP_INK,color);
 }
 cv_blit(0,24);cv_begin(240,18,SP_BG);
 sp_text(12,3,&SP_F7,"TAP: LOAD / HOLD: SAVE",SP_MUT,SP_BG);cv_blit(0,148);sp_keyboard(base,1);
 cv_begin(240,40,SP_PANEL);sp_text(12,7,&SP_F7,"KNOB BANK: NEXT PAD MODE",SP_INK,SP_PANEL);sp_text(12,22,&SP_F7,"SAVE: BOOT / HOLD: PRESETS",SP_MUT,SP_PANEL);cv_blit(0,200);
}
