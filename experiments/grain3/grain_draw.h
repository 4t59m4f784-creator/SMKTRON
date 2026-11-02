static void gr_draw(uint32_t base){
 if(smktron_zoom){uint32_t force=redraw;zm_draw(base);gr_zoom_strip(force);return;}
 uint32_t k,v,live=gr_visual_live();char s[28];uint16_t accent=SP_ACCENT[PV(P_SOUND)];
 static uint32_t last_sig,last_time,was_live;
 uint32_t sig=gr_ctl[0]+gr_ctl[1]*401u+gr_ctl[2]*40501u+gr_ctl[3]*4090501u;
 sig=sig*33u+gr_on+gr_style*2u+touched*11u+by_fx*64u;sig=sig*33u+PV(P_SOUND);
 for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp)sig=sig*33u+voice[v].note;
 uint32_t full=sig!=last_sig||redraw;
 if(!full&&live==was_live&&(!live||fm1_ms-last_time<80u))return;
 last_time=fm1_ms;was_live=live;
 if(full){
  last_sig=sig;sp_header_title(PV(P_SOUND),"GRAIN");
  cv_begin(240,42,SP_BG);cv_oy=-24;
  sp_text(12,28,&SP_F17,"GRAIN",SP_INK,SP_BG);sp_right(228,35,&SP_F8,gr_on?(by_fx?"BYPASS":"ON"):"OFF",accent,SP_BG);
  put(put(s,"K6 STYLE: "),GR_STYLE[gr_style]);sp_text(12,50,&SP_F8,s,accent,SP_BG);cv_oy=0;cv_blit(0,24);
 }
 gr_fragment_stage();
 if(full){
  cv_begin(240,36,SP_BG);cv_oy=-130;sp_text(12,139,&SP_F8,gr_on?"PAD 10: ON":"PAD 10: OFF",SP_MUT,SP_BG);cv_oy=0;cv_blit(0,130);
  sp_keyboard(base,1);
  cv_begin(240,40,SP_PANEL);cv_oy=-200;
  for(k=0;k<4u;k++){int32_t x=(int32_t)k*60;sp_text(x+5,202,&SP_F7,GR_NAMES[k],SP_INK,SP_PANEL);put_int(s,(int32_t)gr_ctl[k],0);tx(x+5,217,&F16,s,SP_INK);}
  cv_oy=0;cv_blit(0,200);
 }
}
