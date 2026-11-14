/* SPDX-License-Identifier: GPL-3.0-only
 * SMKTRON TAPE / WEAR. Read-only UI; no new DSP or audio buffers.
 * WEAR is a parameter illustration, not an oscilloscope. Its dropout lamp
 * reads fxv_drop; its drift reads the first active voice's existing wander.
 */
#define SL_PLATE (smktron_theme?RGB(26,37,32):RGB(205,217,210))
static void sf_label(uint32_t pg)
{
    rrect(12,29,217,29,2,SP_LINE);rrect(13,30,215,27,1,SP_PAPER[0]);
    cv_rect(12,29,5,29,by_lofi?SP_MUT:SP_GREEN);
    sp_text(25,32,&SP_F17,pg==PG_TAPE?"TAPE":"WEAR",SP_INK,SP_PAPER[0]);
    sp_right(219,40,&SP_F7,by_lofi?"BYPASS":"ON",by_lofi?SP_MUT:SP_GREEN,SP_PAPER[0]);
}
static void sf_tape(void)
{
    char count[4];int32_t x,y,fl=ui_lv&&!by_lofi?fxv_flut:0,bow=fl/250;
    if(bow>4)bow=4;if(bow< -4)bow=-4;
    rrect(23,66,194,79,4,SP_LINE);rrect(24,67,192,77,3,SL_PLATE);
    for(x=28;x<=212;x+=184)for(y=71;y<=139;y+=68)disc(x,y,1,SP_MUT);
    thick(68,103,88,129,2,SP_TAPE);thick(152,129,172,103,2,SP_TAPE);
    for(x=88;x<=152;x++)cv_rect(x,129+sn((uint32_t)(x-88)*(1u<<25),bow),1,2,SP_TAPE);
    sp_reel_sized(68,99,reel_a,0,27,1);sp_reel_sized(172,99,reel_a,0,27,1);
    sp_text(91,71,&SP_F7,"TRANSPORT",SP_MUT,SL_PLATE);
    rrect(105,119,31,15,2,SP_INK);
    cv_rect(112,126,17,2,by_lofi?SP_LINE:mix565(SP_GREEN,RGB(180,132,62),(uint32_t)PV(P_SAT)*16u/100u));
    /* A quiet HISS level scale beside the head, not simulated audio noise. */
    for(x=0;!by_lofi&&x<PV(P_HISS)*5/100;x++)cv_rect(94+x*2,123,1,3,SP_MUT);
    count[0]=(char)('0'+counter/100u%10u);count[1]=(char)('0'+counter/10u%10u);
    count[2]=(char)('0'+counter%10u);count[3]=0;
    sp_text(112,136,&SP_F7,count,SP_MUT,SL_PLATE);
}
static void sf_wear(void)
{
    int32_t raw[187],x,py=0,acc=0,yv=0,v,drift=0;
    int32_t age=by_lofi?0:PV(P_AGE),crush=by_lofi?0:PV(P_CRUSH);
    int32_t w=1+age/9,hold_n=1+crush*7/100,q=1+crush/20;
    int32_t drop=ui_lv&&!by_lofi?fxv_drop:0;
    uint32_t phase=reel_a/3u,k;
    for(k=0;!by_lofi&&ui_lv&&k<NV;k++)if(voice[k].amp){
        drift=voice[k].det*PV(P_DRIFT)/32+((voice[k].drift*PV(P_DRIFT))>>6);break;
    }
    rrect(18,66,204,79,4,SP_LINE);rrect(19,67,202,77,3,SL_PLATE);
    sp_text(26,71,&SP_F7,"MASTER",SP_MUT,SL_PLATE);
    sp_text(26,106,&SP_F7,by_lofi?"CLEAN TAPE":"WORN TAPE",SP_MUT,SL_PLATE);
    cv_rect(27,94,187,1,SP_LINE);cv_rect(27,130,187,1,SP_LINE);
    for(x=0;x<187;x++){
        uint32_t ph=(uint32_t)x*0x04000000u-phase;
        int32_t master=sn(ph,8)+sn(ph*3u,2),a=91-master;
        if(x)cv_line(26+x,py,27+x,a,SP_REEL[0]);py=a;
        ph+=(uint32_t)sn((uint32_t)x*0x00800000u,drift)*4000000u;
        raw[x]=sn(ph,8)+sn(ph*3u,2);
    }
    for(x=0;x<187;x++){
        int32_t y;
        acc+=raw[x]-(x>=w?raw[x-w]:0);
        if(x%hold_n==0)yv=acc/(x+1<w?x+1:w)/q*q;
        v=yv;
        /* Only an actual engine dropout makes the dip/marker appear. */
        if(x>=84&&x<=105)v=v*(32768-drop)/32768;
        y=127-v;if(x)thick(26+x,py,27+x,y,2,by_lofi?SP_MUT:SP_GREEN);py=y;
    }
    if(drop>1024){cv_rect(121,116,1,23,RGB(169,113,67));disc(209,110,2,RGB(169,113,67));}
}
static void sf_status(uint32_t sound)
{
    cv_begin(240,18,SP_BG);cv_oy=-148;
    disc(14,155,2,by_lofi?SP_LINE:SP_GREEN);
    sp_text(21,150,&SP_F7,by_lofi?"LOFI OUT":"LOFI IN",by_lofi?SP_MUT:SP_GREEN,SP_BG);
    sp_right(228,150,&SP_F7,MEL_SET_NAME[sound],SP_ACCENT[sound],SP_BG);
    cv_rect(10,164,221,2,SP_INK);cv_oy=0;cv_blit(0,148);
}
static uint32_t sf_on,sf_page,sf_time,sf_sig,sf_active;
static void smktron_lofi_draw(uint32_t base)
{
    uint32_t k,sound=(uint32_t)PV(P_SOUND)%4u,sig=2166136261u;
    int full=!sf_on||sf_page!=page||redraw==0xFFu,active=ui_lv!=0;
    for(k=0;k<4u;k++)sig=(sig^(uint32_t)PV(CTL[page][k].pid))*16777619u;
    sig=(sig^by_lofi)*16777619u;sig=(sig^sound)*16777619u;
    if(full||(redraw&1u))sp_header_title(sound,"LOFI");
    if(full||sig!=sf_sig||active!=(int)sf_active||(active&&fm1_ms-sf_time>=50u)){
        cv_begin(240,124,SP_BG);cv_oy=-24;sf_label(page);
        if(page==PG_TAPE)sf_tape();else sf_wear();
        cv_oy=0;cv_blit(0,24);sf_time=fm1_ms;
    }
    if(full||sig!=sf_sig)sf_status(sound);
    sp_keyboard(base,full||(redraw&4u));
    if(full||sig!=sf_sig||(redraw&8u))sp_parameters_for(page);
    sf_on=1;sf_page=page;sf_sig=sig;sf_active=(uint32_t)active;
}
static void smktron_lofi_leave(void){sf_on=0;}
