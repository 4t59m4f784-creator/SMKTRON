/* SPDX-License-Identifier: GPL-3.0-only
 * SMKTRON EQ / SETUP. Read-only presentation of the original controls.
 * EQ retains the original schematic shelf/bell illustration, not a measured
 * frequency response. No writes to DSP, settings, sequencer or sample memory.
 */
static void ss_label(uint32_t pg)
{
    rrect(12,29,217,29,2,SP_LINE);rrect(13,30,215,27,1,SP_PAPER[0]);
    cv_rect(12,29,5,29,SP_GREEN);
    sp_text(25,32,&SP_F17,PG_NAME[pg],SP_INK,SP_PAPER[0]);
    sp_right(219,40,&SP_F7,pg==PG_EQ?"ALWAYS ON":"SYS",SP_GREEN,SP_PAPER[0]);
}
static int32_t ss_eq_y(int32_t x)
{
    int32_t f=69+PV(P_EQFREQ)*113/15,d=x-f;
    int32_t lo=x<=55?256:x>=95?0:(95-x)*256/40;
    int32_t hi=x<=167?0:x>=207?256:(x-167)*256/40;
    int32_t mid=d*d<1600?256-d*d*256/1600:0;
    int32_t y=104-((PV(P_EQLO)*lo+PV(P_EQMID)*mid+PV(P_EQHI)*hi)*2>>8);
    return y<77?77:y>130?130:y;
}
static void ss_eq(void)
{
    int32_t x,y,py=0,k,mid=69+PV(P_EQFREQ)*113/15;char freq[16];
    rrect(18,67,204,78,4,SP_LINE);rrect(19,68,202,76,3,SL_PLATE);
    for(y=80;y<=128;y+=24)cv_rect(32,y,177,1,y==104?SP_MUT:SP_LINE);
    for(x=56;x<=192;x+=34)cv_rect(x,76,1,55,SP_LINE);
    for(x=32;x<=208;x++){y=ss_eq_y(x);if(x>32)thick(x-1,py,x,y,2,SP_GREEN);py=y;}
    for(k=0;k<3;k++){
        x=k==0?40:k==1?mid:201;y=ss_eq_y(x);
        disc(x,y,3,SP_GREEN);disc(x,y,2,SP_PAPER[0]);
    }
    fmt_val(&CTL[PG_EQ][2],PV(P_EQFREQ),freq);
    sp_text(32,134,&SP_F7,"LOW",SP_MUT,SL_PLATE);
    sp_text(120-sp_width(&SP_F7,freq)/2,134,&SP_F7,freq,SP_MUT,SL_PLATE);
    sp_right(208,134,&SP_F7,"HIGH",SP_MUT,SL_PLATE);
}
/* Same integer A4 conversion as the original SETUP screen. */
static int32_t ss_hz10(void)
{
    int32_t tune=PV(P_TUNE);
    return tune>=0?(int32_t)(((MEL_POW2[tune]>>14)*4400u)>>16)
        :(int32_t)(((MEL_POW2[1200+tune]>>14)*4400u)>>17);
}
static void ss_setup(uint32_t beat)
{
    static const char *const label[4]={"TEMPO","TUNING","TAPE LENGTH","SCREEN"};
    int32_t k,hz10=ss_hz10();char value[28],*p;
    for(k=0;k<4;k++){
        int32_t y=69+k*18;uint16_t bg=SP_BG;
        if((uint32_t)k==touched){bg=(smktron_theme?RGB(34,53,43):RGB(211,224,216));rrect(12,y-1,217,17,2,bg);cv_rect(12,y+3,2,8,SP_GREEN);}
        switch(k){
        case 0:put(put_int(value,PV(P_BPM),0)," BPM");break;
        case 1:p=put_int(put(value,"A4 "),hz10/10,0);*p++='.';*p++=(char)('0'+hz10%10);put(p," Hz");break;
        case 2:put(value,PV(P_LEN)?"8 SECONDS":"AS HELD");break;
        default:put(value,PV(P_LOOK)?"TAPE / ARP-SEQ":"CLEAN / ARP-SEQ");break;
        }
        sp_text(16,y+2,&SP_F7,label[k],SP_MUT,bg);
        sp_right(224,y,&SP_F10,value,SP_INK,bg);cv_rect(16,y+15,209,1,SP_LINE);
    }
    disc(18,145,2,beat?SP_GREEN:SP_LINE);
    sp_text(25,141,&SP_F7,"TEMPO",SP_MUT,SP_BG);
}
static void ss_status(uint32_t sound,uint32_t pg)
{
    static const char *const hint[4]={"ARP / SEQ TEMPO","A4 FINE TUNING","NOTE TAPE LENGTH","LOOK: ARP / SEQ"};
    const char *s=pg==PG_EQ?"EQ IN":touched<4u?hint[touched]:"GLOBAL SETTINGS";
    cv_begin(240,18,SP_BG);cv_oy=-148;
    disc(14,155,2,SP_GREEN);sp_text(21,150,&SP_F7,s,SP_MUT,SP_BG);
    sp_right(228,150,&SP_F7,MEL_SET_NAME[sound],SP_ACCENT[sound],SP_BG);
    cv_rect(10,164,221,2,SP_INK);cv_oy=0;cv_blit(0,148);
}
static void ss_appearance(uint32_t base){
 static uint32_t previous=0xFFFFFFFFu;
 uint32_t sig=((uint32_t)set.bypass>>3)|(smktron_output<<8);
 if(previous==sig&&!(redraw&11u)){sp_keyboard(base,redraw&4u);return;}previous=sig;
 char value[12];uint32_t k;
 sp_header_title(0,"GLOBAL 2 / 2");
 cv_begin(240,124,SP_BG);cv_oy=-24;ss_label(PG_APPEAR);
 for(k=0;k<3u;k++){
  int32_t y=70+(int32_t)k*25;
  sp_text(16,y,&SP_F8,k==0?"PAD BRIGHTNESS":k==1?"THEME":"MASTER",SP_MUT,SP_BG);
  if(k==1)put(value,smktron_theme?"DARK":"LIGHT");
  else put(put_int(value,k==0?smktron_pad_scale*25:(int32_t)smktron_output,0),"%");
  sp_right(223,y-2,&SP_F14,value,SP_INK,SP_BG);cv_rect(16,y+19,207,1,SP_LINE);
 }
 cv_oy=0;cv_blit(0,24);
 cv_begin(240,18,SP_BG);sp_text(16,3,&SP_F7,"GLOBE: NEXT GLOBAL PAGE",SP_MUT,SP_BG);cv_blit(0,148);
 sp_keyboard(base,1);cv_begin(240,40,SP_PANEL);
 for(k=0;k<3u;k++){
  int32_t x=8+(int32_t)k*79;
  sp_text(x,5,&SP_F7,k==0?"PAD LIGHT":k==1?"THEME":"MASTER",SP_MUT,SP_PANEL);
  if(k==1)put(value,smktron_theme?"DARK":"LIGHT");
  else put(put_int(value,k==0?smktron_pad_scale*25:(int32_t)smktron_output,0),"%");
  sp_text(x,19,&SP_F14,value,SP_INK,SP_PANEL);
 }cv_blit(0,200);
}
static uint32_t ss_on,ss_page,ss_sig,ss_beat;
static void smktron_settings_draw(uint32_t base)
{
    if(page==PG_APPEAR){ss_appearance(base);return;}
    uint32_t k,sound=(uint32_t)PV(P_SOUND)%4u,sig=2166136261u;
    uint32_t beat=(fm1_ms%(60000u/(uint32_t)PV(P_BPM)))<80u;
    int full=!ss_on||ss_page!=page||redraw==0xFFu;
    for(k=0;k<4u;k++)sig=(sig^(uint32_t)PV(CTL[page][k].pid))*16777619u;
    sig=(sig^sound)*16777619u;sig=(sig^touched)*16777619u;
    if(full||(redraw&1u))sp_header_title(sound,page==PG_EQ?"EQUALIZER":"GLOBAL 1 / 2");
    if(full||sig!=ss_sig||(page==PG_SETUP&&beat!=ss_beat)){
        cv_begin(240,124,SP_BG);cv_oy=-24;ss_label(page);
        if(page==PG_EQ)ss_eq();else ss_setup(beat);
        cv_oy=0;cv_blit(0,24);
    }
    if(full||sig!=ss_sig)ss_status(sound,page);
    sp_keyboard(base,full||(redraw&4u));
    if(full||sig!=ss_sig||(redraw&8u))sp_parameters_for(page);
    ss_on=1;ss_page=page;ss_sig=sig;ss_beat=beat;
}
static void smktron_settings_leave(void){ss_on=0;}
