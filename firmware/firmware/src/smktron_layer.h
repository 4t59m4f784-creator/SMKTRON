/* SPDX-License-Identifier: GPL-3.0-only
 * SMKTRON LAYER. Read-only presentation of existing P_BSND, P_BMIX,
 * P_BOCT and P_BDET; shares fonts, palette and keyboard with PLAY.
 */
static void sl_label(int32_t x,const char *role,const char *sound,uint32_t bank,int enabled)
{
    uint16_t paper=enabled?SP_PAPER[bank]:(smktron_theme?RGB(33,42,36):RGB(220,225,220)),ink=enabled?SP_INK:SP_MUT;
    rrect(x,29,103,34,2,SP_LINE);rrect(x+1,30,101,32,1,paper);
    cv_rect(x,29,5,34,enabled?SP_ACCENT[bank]:SP_LINE);
    sp_text(x+11,32,&SP_F7,role,SP_MUT,paper);
    sp_text(x+11,43,&SP_F13,sound,ink,paper);
}
static void sl_stage(uint32_t sound,uint32_t layer)
{
    uint32_t second=layer?(layer-1u)%4u:0u;
    char left[16],right[16];int32_t mix=layer?PV(P_BMIX):0;
    cv_begin(240,124,SP_BG);cv_oy=-24;
    sl_label(12,"MAIN",MEL_SET_NAME[sound],sound,1);
    sl_label(126,"LAYER",layer?MEL_SET_NAME[second]:"OFF",second,layer!=0u);
    thick(62,109,83,130,2,SP_TAPE);thick(83,130,157,130,2,SP_TAPE);thick(157,130,178,109,2,SP_TAPE);
    sp_reel_sized(62,98,reel_a,sound,29,1);
    sp_reel_sized(178,98,layer?reel_a:0u,second,29,layer!=0u);
    rrect(105,122,31,15,2,SP_INK);cv_rect(111,129,19,2,SP_GREEN);
    put(put_int(put(put(left,MEL_SET_NAME[sound])," "),100-mix,0),"%");
    if(layer)put(put_int(put(put(right,MEL_SET_NAME[second])," "),mix,0),"%");
    else put(right,"LAYER OFF");
    sp_text(12,138,&SP_F7,left,SP_ACCENT[sound],SP_BG);
    sp_right(228,138,&SP_F7,right,layer?SP_ACCENT[second]:SP_MUT,SP_BG);
    cv_oy=0;cv_blit(0,24);
}
static void sl_blend(uint32_t sound,uint32_t layer)
{
    int32_t mix=layer?PV(P_BMIX):0,cut=12+(100-mix)*216/100;
    uint32_t second=layer?(layer-1u)%4u:0u;
    cv_begin(240,18,SP_BG);cv_oy=-148;
    rrect(12,151,217,8,2,layer?SP_ACCENT[second]:SP_LINE);
    if(cut>12)rrect(12,151,cut-12+1,8,2,SP_ACCENT[sound]);
    if(layer)cv_rect(cut-1,149,2,12,SP_WHITE);
    cv_rect(10,164,221,2,SP_INK);
    cv_oy=0;cv_blit(0,148);
}
static uint32_t sl_on,sl_last_sig,sl_angle,sl_time;
static void smktron_layer_draw(uint32_t base)
{
    uint32_t sound=(uint32_t)PV(P_SOUND)%4u,layer=(uint32_t)PV(P_BSND);
    uint32_t sig=sound|(layer<<3)|((uint32_t)PV(P_BMIX)<<6)|((uint32_t)(PV(P_BOCT)+1)<<13)|((uint32_t)PV(P_BDET)<<15);
    int full=!sl_on||redraw==0xFFu,changed_state=sig!=sl_last_sig;
    if(full||(redraw&1u))sp_header_title(sound,"LAYER");
    if(full||changed_state||(sl_angle!=reel_a&&fm1_ms-sl_time>=50u)){
        sl_stage(sound,layer);sl_angle=reel_a;sl_time=fm1_ms;
    }
    if(full||changed_state)sl_blend(sound,layer);
    sp_keyboard(base,full||(redraw&4u));
    if(full||changed_state||(redraw&8u))sp_parameters_for(PG_LAYER);
    sl_on=1;sl_last_sig=sig;
}
static void smktron_layer_leave(void){sl_on=0;}
