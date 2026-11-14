/* SPDX-License-Identifier: GPL-3.0-only. Existing SMKTRON fonts and canvas only. */
static void mm_text(int x,int y,const char *s,uint16_t c){sp_text(x,y,&SP_F8,s,c,SP_BG);}
static void mm_large(int x,int y,const char *s,uint16_t c){sp_text(x,y,&SP_F13,s,c,SP_BG);}
static void mm_heading(const char *title,const char *detail){
 sp_text(12,7,&SP_F17,title,SP_INK,SP_BG);mm_large(12,35,detail,SP_GREEN);cv_rect(12,56,216,2,SP_GREEN);
}
static void mm_cell(unsigned k,const char *label,const char *value,int enabled){
 int x=12+(int)(k%2u)*110,y=64+(int)(k/2u)*64;uint16_t ink=enabled?SP_INK:SP_MUT;
 cv_rect(x,y,106,60,SP_LINE);cv_rect(x+1,y+1,104,58,SP_PANEL);
 sp_text(x+7,y+4,smktron_zoom?&SP_F17:&SP_F13,label,ink,SP_PANEL);
 if(smktron_zoom)tx(x+7,y+29,tw(&F24,value)<=92?&F24:&F20,value,ink);
 else sp_text(x+7,y+30,&SP_F17,value,ink,SP_PANEL);
}
static int mm_same(const char *a,const char *b){while(*a&&*a==*b){a++;b++;}return *a==*b;}
static const char *mm_hint(const char *normal){
 if(mm_exiting)return "RETURNING TO TAPE";
 if(mm_same(mm_status,"SAVED")&&!mm_dirty)return mm_status;
 if(mm_same(mm_status,"SAVE FAILED / RETRY")||mm_same(mm_status,"RELEASE KEYS / PEDAL")||mm_same(mm_status,"MIDI PANIC"))return mm_status;
 return normal;
}
/* Draw the same absolute layout into two clipped tiles: no extra framebuffer. */
static void mm_sheet(unsigned entry){
 char b[48],v[24];
 for(unsigned half=0;half<2;half++){
  cv_begin(240,120,SP_BG);cv_oy=-(int)half*120;
  if(entry){
   mm_heading("MIDI MODE","SMKTRON / GLOBAL");
   cv_rect(12,68,216,61,SP_PANEL);
   sp_text(24,75,smktron_zoom?&SP_F17:&SP_F13,"K1 MODE",SP_MUT,SP_PANEL);
   tx(24,101,&F24,"SYNTH",SP_INK);
   sp_text(12,145,&SP_F17,"K1 RIGHT",SP_GREEN,SP_BG);
   sp_text(12,171,&SP_F17,"ENTER MIDI MODE",SP_INK,SP_BG);
   mm_large(12,204,"GLOBE NEXT / K6 PAGE",SP_MUT);
  }else if(mm_reset_ask){
   mm_heading("RESET MAP?","MIDI CONTROLLER");
   sp_text(12,80,&SP_F17,"RESTORE DEFAULTS",SP_INK,SP_BG);
   sp_text(12,127,&SP_F17,"BT: CONFIRM",SP_GREEN,SP_BG);
   sp_text(12,165,&SP_F17,"PATCH: CANCEL",SP_INK,SP_BG);
  }else if(mm_settings){
   mm_heading("MIDI GLOBAL","K1 LEFT: SYNTH");
   mm_cell(0,"K1 MODE","MIDI",1);
   fmt_u(v,mm_data.selected+1);mm_cell(1,"K2 MAP",v,1);
   fmt_u(v,mm_main_ch());mm_cell(2,"K3 CH",v,1);
   mm_cell(3,"K4 KNOBS",mm_relative()?"REL":"ABS",1);
   mm_large(12,199,"BT SAVE / REC RESET",SP_INK);
   mm_large(12,221,mm_hint("GLOBE BACK"),SP_MUT);
  }else if(mm_pick){
   mm_heading("EDIT MIDI","SELECT A CONTROL");
   sp_text(12,81,&SP_F17,"TOUCH A CONTROL",SP_GREEN,SP_BG);
   mm_large(12,118,"KNOB / FADER",SP_INK);
   mm_large(12,146,"PAD / BUTTON / WHEEL",SP_INK);
   mm_large(12,199,"BT SAVE",SP_INK);
   mm_large(12,221,mm_hint("PARA BACK"),SP_MUT);
  }else{
   mm_entry_t *e=&mm_profile()->map[mm_sel];unsigned pad=mm_sel>=MM_PAD&&mm_sel<MM_BTN;
   mm_name(mm_sel,v);put(b,"K5: ");scat(b,v);if(mm_dirty)scat(b," *");mm_heading("EDIT MIDI",b);
   static const char *const type[]={"OFF","CC","TOGGLE","NOTE","BEND"};
   mm_cell(0,"K1 TYPE",type[e->type],1);
   unsigned numbered=e->type!=MM_OFF&&e->type!=MM_BEND;
   if(numbered)fmt_u(v,e->num);else put(v,"--");mm_cell(1,e->type==MM_NOTE?"K2 NOTE":e->type==MM_CC||e->type==MM_TOGGLE?"K2 CC":"K2 NUM",v,numbered);
   if(e->ch)fmt_u(v,e->ch);else put(v,"MAIN");mm_cell(2,"K3 CH",v,e->type!=MM_OFF);
   mm_cell(3,"K4 COLOR",pad?MM_COLOR_NAME[mm_profile()->color[mm_sel-MM_PAD]]:"--",pad);
   mm_large(12,199,"K5 SELECT / PATCH PICK",SP_INK);
   mm_large(12,221,mm_hint("BT SAVE / PARA BACK"),SP_MUT);
  }
  cv_oy=0;cv_blit(0,half*120);
 }
}
static void mm_entry_draw(void){if(redraw)mm_sheet(1);}
static void mm_draw(void){
 unsigned blink=mm_edit&&!mm_pick&&mm_sel>=MM_PAD&&mm_sel<MM_BTN;
 if(!mm_screen_dirty&&(!blink||fm1_ms-mm_draw_t<200u))return;if(fm1_ms-mm_draw_t<40u)return;
 mm_screen_dirty=0;mm_draw_t=fm1_ms;
 if(mm_settings||mm_edit||mm_reset_ask){mm_sheet(0);return;}
 char b[48],v[24];uint16_t amber=smktron_theme?RGB(215,163,87):RGB(133,87,29);
 cv_oy=0;cv_begin(240,120,SP_BG);mm_text(12,4,"SMKTRON",SP_MUT);mm_text(161,4,"USB + TRS",amber);
 cv_rect(12,23,216,2,SP_GREEN);sp_text(12,30,&SP_F17,"MIDI MODE",SP_INK,SP_BG);
 put(b,"MAP ");fmt_u(b+4,mm_data.selected+1);scat(b,mm_dirty?" *  CH ":"    CH ");fmt_u(b+slen(b),mm_main_ch());mm_large(12,58,b,SP_GREEN);
 mm_name(mm_last,b);sp_text(12,79,&SP_F13,b,SP_INK,SP_BG);fmt_u(v,mm_last_v);if(smktron_zoom)tx(228-tw(&F24,v),75,&F24,v,SP_GREEN);else sp_right(228,77,&SP_F17,v,SP_GREEN,SP_BG);b[0]=0;
 if(mm_last<MM_N){mm_entry_t *e=&mm_profile()->map[mm_last];scat(b,MM_TYPE_NAME[e->type]);if(e->type!=MM_BEND){scat(b," ");fmt_u(b+slen(b),e->num);}scat(b," CH ");fmt_u(b+slen(b),mm_channel(e)+1);}else{scat(b,"NOTE ");fmt_u(b+5,mm_last_v);}mm_large(12,102,b,amber);
 cv_blit(0,0);cv_begin(240,120,SP_BG);
 for(unsigned k=0;k<8;k++){unsigned x=12+k*27,h=mm_values[8*mm_kbank+k]*24u/127u;cv_rect(x,7,18,26,SP_PANEL);cv_rect(x,32-h,18,h+1,SP_GREEN);}
 for(unsigned p=0;p<16;p++){unsigned i=MM_PAD+16*mm_pbank+p,c=mm_pad_colour(i),x=12+p%8*27,y=43+p/8*21;const uint8_t *rgb=MM_COLOR[c];uint16_t col=RGB(rgb[0],rgb[1],rgb[2]);cv_rect(x,y,18,15,mix565(SP_PANEL,col,mm_toggle[i]||(mm_pad_down>>p&1u)?16:6));}
 put(b,"K ");scat(b,mm_kbank?"B":"A");scat(b," F ");scat(b,mm_fbank?"B":"A");scat(b," P ");scat(b,mm_pbank?"B":"A");scat(b," OCT ");fmt_s(b+slen(b),mm_oct);mm_large(12,80,b,SP_MUT);
 sp_text(12,104,&SP_F10,mm_status,SP_INK,SP_BG);cv_blit(0,120);
}
