/* SPDX-License-Identifier: GPL-3.0-only */
static uint32_t ch_ui_on,ch_ui_last;
static void ch_ui_leave(void){ch_ui_on=0;}
static void ch_pan_text(char *text,int32_t value){
 if(!value)put(text,"CENTER");else put_int(put(text,value<0?"L":"R"),value<0?-value:value,0);
}
static void ch_draw(uint32_t base){
 uint32_t i,j;char text[40];
 if(ch_ui_on&&!redraw&&fm1_ms-ch_ui_last<100u)return;
 ch_ui_on=1;ch_ui_last=fm1_ms;
 sp_header_title((uint32_t)PV(P_SOUND),"GLOBAL 3 / 3");
 cv_begin(240,124,SP_BG);cv_oy=-24;
 ss_label(PG_CHORD);
 sp_text(16,63,&SP_F7,MEL_SET_NAME[(uint32_t)PV(P_SOUND)],SP_INK,SP_BG);
 sp_right(224,63,&SP_F7,MEL_SET_NAME[ch_sound],SP_INK,SP_BG);
 sp_text(16,75,&SP_F7,"MELODY",SP_MUT,SP_BG);
 sp_right(224,75,&SP_F7,"CHORDS",SP_MUT,SP_BG);
 cv_rect(16,85,208,4,SP_LINE);cv_rect(16,85,(int32_t)(208u*(100u-ch_mix)/100u),4,SP_GREEN);
 for(i=0;i<8u;i++){
  int32_t x=14+(int32_t)(i%4u)*55,y=95+(int32_t)(i/4u)*23;
  uint16_t bg=i==ch_active?SP_GREEN:SP_PAPER[0];
  rrect(x,y,50,19,2,bg);put_int(text,(int32_t)i+1,0);
  sp_text(x+4,y+3,&SP_F10,text,SP_INK,bg);
  if(ch_rec&&i==ch_target)put(text,"REC");else if(ch_bank.count[i])put(put_int(text,ch_bank.count[i],0),"N");else put(text,"--");
  sp_right(x+46,y+6,&SP_F7,text,SP_INK,bg);
 }
 cv_oy=0;cv_blit(0,24);
 cv_begin(240,18,SP_BG);
 sp_text(12,3,&SP_F7,ch_rec?(ch_target<8u?"PLAY CHORD, RELEASE ALL":"REC: CHOOSE PAD 1-8"):(ch_hold?"HOLD: NEXT PAD REPLACES":"HOLD OFF: RELEASE STOPS"),SP_MUT,SP_BG);cv_blit(0,148);
 sp_keyboard(base,1);cv_begin(240,40,SP_PANEL);
 for(j=0;j<4u;j++){
  int32_t x=8+(int32_t)j*58;
  sp_text(x,5,&SP_F7,j==0?"MIX":j==1?"PAN MEL":j==2?"PAN CHD":"MODE",SP_MUT,SP_PANEL);
  if(j==0)put(put_int(text,(int32_t)ch_mix,0),"%");
  else if(j<3u)ch_pan_text(text,j==1?ch_pan_m:ch_pan_c);else put(text,ch_on?"ON":"OFF");
  sp_text(x,21,&SP_F8,text,SP_INK,SP_PANEL);
 }cv_blit(0,200);
}
