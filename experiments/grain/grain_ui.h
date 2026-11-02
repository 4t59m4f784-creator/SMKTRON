/* SPDX-License-Identifier: GPL-3.0-only. Experimental values are session-only. */
static const char *const GR_NAMES[]={"SIZE","SPREAD","DENSITY","MIX"};
static void gr_knob(uint32_t k,int32_t d){
 static const int16_t lo[]={20,0,15,0},hi[]={400,100,100,100};
 int32_t v=(int32_t)gr_ctl[k]+d;if(v<lo[k])v=lo[k];if(v>hi[k])v=hi[k];
 gr_ctl[k]=(uint32_t)v;touched=k;redraw=0xFFu;
}
static void gr_draw(uint32_t base){
 uint32_t row,k,v;char s[28];uint16_t accent=SP_ACCENT[PV(P_SOUND)];
 static uint32_t last;uint32_t sig=gr_ctl[0]+gr_ctl[1]*401u+gr_ctl[2]*40501u+gr_ctl[3]*4090501u;
 sig=sig*33u+PV(P_SOUND);for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp)sig=sig*33u+voice[v].note;
 if(sig==last&&!redraw)return;last=sig;
 sp_header_title(PV(P_SOUND),"GRAIN / GR1");
 cv_begin(240,42,SP_BG);cv_oy=-24;
 sp_text(12,28,&SP_F17,"GRAIN",SP_INK,SP_BG);sp_right(228,35,&SP_F8,gr_ctl[3]?"ON":"DRY",accent,SP_BG);
 sp_text(12,50,&SP_F8,"TAPE FRAGMENTS",SP_MUT,SP_BG);cv_oy=0;cv_blit(0,24);
 for(row=0;row<2;row++){
  cv_begin(240,64,SP_BG);cv_oy=-(66+(int32_t)row*64);
  for(k=row*2;k<row*2+2;k++){
   int32_t x=8+(k%2)*116,y=66+row*64;
   cv_rect(x,y,108,60,touched==k?accent:SP_LINE);cv_rect(x+1,y+1,106,58,SP_PANEL);
   sp_text(x+7,y+3,&SP_F13,GR_NAMES[k],SP_INK,SP_PANEL);
   put(put_int(s,(int32_t)gr_ctl[k],0),k?"%":" ms");tx(x+7,y+28,&F20,s,SP_INK);
   put_int(put(s,"K"),k+1,0);sp_right(x+102,y+49,&SP_F7,s,accent,SP_PANEL);
  }cv_oy=0;cv_blit(0,66+row*64);
 }
 cv_begin(240,30,SP_BG);cv_oy=-194;
 sp_text(8,200,&SP_F8,"ARP REPLACED / SEQ AVAILABLE",SP_MUT,SP_BG);cv_oy=0;cv_blit(0,194);
 /* Existing compact keyboard renderer; only its strip is reused. */
 cv_begin(240,16,SP_BG);cv_oy=-224;int32_t white=0;base-=5u;
 for(k=0;k<37;k++)if(!black(48+k)){int lit=0;for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp&&voice[v].note==base+k)lit=1;cv_rect(8+white*224/22,224,9,13,lit?RGB(255,144,32):SP_WHITE);white++;}
 white=0;for(k=0;k<37;k++){if(black(48+k)){int lit=0;for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].amp&&voice[v].note==base+k)lit=1;cv_rect(8+white*224/22-3,224,6,8,lit?RGB(255,144,32):SP_BLACK);}else white++;}
 cv_oy=0;cv_blit(0,224);
}
