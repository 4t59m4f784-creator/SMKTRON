/* Zoom is a presentation-only layer. Reuses existing fonts/canvas; no DSP writes. */
#define Z_INK RGB(238,243,238)
static void zm_value(uint32_t k,char *v){
 if(page==PG_APPEAR){if(k==1)put(v,"ZOOM");else if(k==3)put(v,"--");else put(put_int(v,k==0?smktron_pad_scale*25:smktron_output,0),"%");}
 else if(page==PG_CHORD){if(k==0)put(put_int(v,ch_mix,0),"%");else if(k<3)ch_pan_text(v,k==1?ch_pan_m:ch_pan_c);else put(v,ch_on?"ON":"OFF");}
 else fmt_val(&CTL[page][k],page==PG_SEQ&&k==2?(int32_t)seq_cur:PV(CTL[page][k].pid),v);
}
static void zm_draw(uint32_t base){
 static uint32_t last;uint32_t i,k,v,st=PV(P_SOUND),ly=PV(P_BSND),sig=page;char s[48],value[24];
 uint16_t accent=SP_GREEN;const sp_font_t *f=&SP_F17;
 for(i=0;i<NP;i++)sig=sig*33u+(uint32_t)PV(i);
 sig=sig*33u+touched;sig=sig*33u+smktron_output;sig=sig*33u+smktron_pad_scale;
 sig=sig*33u+ch_mix+ch_pan_m*101u+ch_pan_c*10001u+ch_on*17u+ch_rec*31u+ch_target*43u+ch_active*61u+ch_hold*71u;
 sig=sig*33u+seq_step+seq_play*32u+seq_rec*64u+arp_on*128u+arp_n*256u+by_fx*65536u+by_lofi*131072u;
 for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp)sig=sig*33u+voice[v].note;
 if(sig==last&&!redraw)return;last=sig;
 sp_header_title(st,PG_NAME[page]);
 cv_begin(240,42,SP_BG);cv_oy=-24;
 if(page==PG_PLAY){accent=SP_ACCENT[st];rrect(8,27,224,35,3,SP_PAPER[st]);cv_rect(8,29,3,28,accent);sp_text(18,28,&SP_F17,MEL_SET_NAME[st],accent,SP_PAPER[st]);put(s,"PLAY / TAPE ");put_int(s+12,st+1,0);}
 else if(page==PG_LAYER&&ly){
  const char *a=MEL_SET_NAME[st],*b=MEL_SET_NAME[ly-1u];
  int32_t x=18;if(sp_width(f,a)+sp_width(f," + ")+sp_width(f,b)>207)f=&SP_F13;
  rrect(8,27,224,35,3,SP_PANEL);cv_rect(8,29,3,13,SP_ACCENT[st]);cv_rect(8,44,3,13,SP_ACCENT[ly-1u]);
  sp_text(x,29,f,a,SP_ACCENT[st],SP_PANEL);x+=sp_width(f,a);sp_text(x,29,f," + ",Z_INK,SP_PANEL);x+=sp_width(f," + ");sp_text(x,29,f,b,SP_ACCENT[ly-1u],SP_PANEL);put(s,"MAIN + LAYER");
 }else{cv_rect(8,29,3,28,accent);sp_text(18,28,page==PG_CHORD?&SP_F13:&SP_F17,page==PG_LAYER?MEL_SET_NAME[st]:PG_NAME[page],Z_INK,SP_BG);put(s,page==PG_LAYER?"LAYER OFF":MEL_SET_NAME[st]);}
 sp_text(18,50,&SP_F8,s,SP_MUT,page==PG_PLAY?SP_PAPER[st]:page==PG_LAYER&&ly?SP_PANEL:SP_BG);cv_oy=0;cv_blit(0,24);
 for(i=0;i<2;i++){
  cv_begin(240,64,SP_BG);cv_oy=-(66+(int32_t)i*64);
  for(k=i*2;k<i*2+2;k++){
   int32_t x=8+(k%2)*116,y=66+i*64;const char *name=CTL[page][k].name;
   if(page==PG_APPEAR){static const char *const n[]={"PAD LIGHT","THEME","MASTER","--"};name=n[k];}
   if(page==PG_CHORD){static const char *const n[]={"MIX","PAN MEL","PAN CHD","MODE"};name=n[k];}
   cv_rect(x,y,108,60,touched==k?accent:SP_LINE);cv_rect(x+1,y+1,106,58,SP_PANEL);
   if(touched==k)cv_rect(x,y,2,60,accent);
   f=sp_width(&SP_F17,name)>96?&SP_F13:&SP_F17;sp_text(x+7,y+2,f,name,Z_INK,SP_PANEL);
   zm_value(k,value);tx(x+7,y+29,tw(&F20,value)<=94?&F20:&F16,value,Z_INK);
   put_int(put(s,"K"),k+1,0);sp_right(x+102,y+49,&SP_F7,s,accent,SP_PANEL);
  }cv_oy=0;cv_blit(0,66+i*64);
 }
 cv_begin(240,46,SP_BG);cv_oy=-194;
 if(page==PG_SEQ){for(i=0;i<16;i++)cv_rect(8+i*14,197,11,7,i==seq_step&&seq_play?RGB(255,144,32):i<(uint32_t)PV(P_SLEN)?SP_GREEN:SP_LINE);put(s,seq_rec?"REC":seq_play?"PLAY":"STOP");}
 else if(page==PG_CHORD){if(ch_rec){put(s,ch_target<8?"REC: PLAY CHORD, RELEASE KEYS":"REC: CHOOSE PAD 1-8");}else{put(s,ch_on?(ch_hold?"ON / HOLD ON":"ON / HOLD OFF"):"MODE OFF");}}
 else if(page==PG_ARP){put(put_int(put(s,arp_on?"ARP ON / ":"ARP OFF / "),PV(P_BPM),0)," BPM");}
 else put(s,page==PG_TAPE||page==PG_WEAR?(by_lofi?"LOFI BYPASS":"LOFI ON"):page>=PG_VIB&&page<=PG_VERB?(by_fx?"FX BYPASS":PV(CTL[page][3].pid)?"FX ON":"FX OFF"):"READY");
 sp_text(8,page==PG_SEQ?207:197,&SP_F8,s,SP_MUT,SP_BG);
 /* Compact C..C keyboard, orange activity, without a second large canvas. */
 {int32_t white=0,wx[37];uint8_t lit[37];base-=5u;
 for(k=0;k<37;k++){lit[k]=0;for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].note==base+k&&voice[v].amp)lit[k]=1;wx[k]=8+white*224/22;if(!black(48+k))white++;}
 for(i=0;i<2;i++)for(k=0;k<37;k++)if((uint32_t)black(48+k)==i)cv_rect(wx[k]-(i?3:0),224,i?6:9,i?8:13,lit[k]?RGB(255,144,32):i?SP_BLACK:SP_WHITE);
 }
 cv_oy=0;cv_blit(0,194);
}
