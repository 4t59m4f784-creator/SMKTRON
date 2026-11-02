/* SPDX-License-Identifier: GPL-3.0-only
 * Procedural boot art: existing fonts/reels, no images or extra audio state. */
static uint32_t boot_start,boot_frame,boot_done;
static uint32_t boot_elapsed(void){return fm1_ms-boot_start;}
static void boot_draw(void){
 uint32_t t,i,first=!boot_frame,theme=smktron_theme;
 if(boot_done)return;
 if(first)boot_start=fm1_ms;
 t=boot_elapsed();
 if(t>=1400u||ui_lv||dirty||ch_on||page!=PG_PLAY){boot_done=1;redraw=255u;lcd_fill(0,0,240,240,SP_BG);return;}
 if(boot_frame && t/70u==boot_frame-1u)return;
 boot_frame=t/70u+1u;smktron_theme=1;
 if(first){
  lcd_fill(0,0,240,240,SP_BG);
  cv_begin(240,70,SP_BG);
  sp_text((240-sp_width(&SP_F7,"TAPE INSTRUMENT"))/2,18,&SP_F7,"TAPE INSTRUMENT",RGB(150,169,137),SP_BG);
  sp_text((240-sp_width(&SP_F17,"SMKTRON"))/2,38,&SP_F17,"SMKTRON",RGB(240,230,196),SP_BG);cv_blit(0,0);
  cv_begin(240,40,SP_BG);sp_text((240-sp_width(&SP_F7,"STARTING"))/2,5,&SP_F7,"STARTING",RGB(195,139,65),SP_BG);cv_blit(0,193);
 }
 cv_begin(240,120,SP_BG);
 thick(61,58,85,105,2,RGB(173,123,60));thick(85,105,155,105,2,RGB(173,123,60));thick(155,105,179,58,2,RGB(173,123,60));
 sp_reel_sized(61,53,t*1500000u,0,40,1);sp_reel_sized(179,53,t*1500000u,0,40,1);
 rrect(106,98,28,14,2,RGB(184,193,164));cv_rect(112,104,16,2,SP_BG);cv_blit(0,70);
 cv_begin(240,6,SP_BG);for(i=0;i<8;i++)cv_rect(80+(int32_t)i*10,0,7,3,i<=t*8u/1400u?RGB(150,169,137):SP_LINE);cv_blit(0,217);
 smktron_theme=theme;
}
static int boot_leds(void){
 uint32_t t=boot_elapsed(),p,v,dist,head;
 if(boot_done||t>=1400u)return 0;
 head=t*19u/1100u;
 for(p=0;p<16u;p++){
  dist=head>p?head-p:p-head;v=dist<4u?100u-dist*25u:0u;
  if(t>=1100u)v=(1400u-t)/3u;
  if(p<8u)padled_set(p,v*3u/5u,v,v*3u/4u);
  else padled_set(p,v,v*3u/5u,v/5u);
 }
 padled_flush(); /* Native LEDs require committing the buffered colours. */
 return 1;
}

#if defined(DINGHY_WEB)
EXPORT void web_boot_delay(uint32_t ms){fm1_ms+=ms;}
#endif
