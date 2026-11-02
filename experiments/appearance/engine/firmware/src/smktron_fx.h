/* SPDX-License-Identifier: GPL-3.0-only
 * SMKTRON effects. Read-only presentation; uses existing DSP phases and
 * sparse buffer energy for activity/trails. Echo/room drawings are schematic,
 * not audio waveforms or acoustic simulations. No DSP writes or new buffers.
 */
static uint32_t sx_on,sx_page,sx_time,sx_sig,sx_phase;
static uint32_t sx_energy(uint32_t pg)
{
    uint32_t i,total=0,count=0;
#if !SMKTRON_PLATE
    uint32_t k;
#endif
    if(pg==PG_ECHO){
        if(!PV(P_EMIX))return 0;
        for(i=0;i<ECHO_N;i+=689u){int32_t v=echo_buf[i];total+=(uint32_t)(v<0?-v:v);count++;}
    }else if(pg==PG_VERB){
        if(!PV(P_RMIX))return 0;
#if SMKTRON_PLATE
        for(i=0;i<REV_N;i+=389u){int32_t v=ovp_fx_mem[i];total+=(uint32_t)(v<0?-v:v);count++;}
#else
        for(k=0;k<4u;k++)for(i=0;i<RV_CL[k];i+=83u){
            int32_t v=rv_c[k][i];uint32_t a=v<0?0u-(uint32_t)v:(uint32_t)v;
            total+=a>32767u?32767u:a;count++;
        }
#endif
    }
    /* Ignore the very small fixed-point residue, rather than animate forever. */
    if(!count)return 0;total=total/count/8u;if(total<=3u)return 0;
    total-=3u;return total>256u?256u:total;
}
static int32_t sx_mix(uint32_t pg)
{
    int32_t mix=PV(CTL[pg][3].pid),mod=p_mod*100/127;
    if(pg==PG_VIB&&mod>mix)mix=mod;
    return mix;
}
static void sx_label(uint32_t pg,int32_t mix)
{
    const char *state=by_fx?"BYPASS":mix?"ON":"OFF";
    if(pg==PG_VIB&&!by_fx&&p_mod*100/127>PV(P_VMIX))state="MOD";
    rrect(12,29,217,29,2,SP_LINE);rrect(13,30,215,27,1,SP_PAPER[0]);
    cv_rect(12,29,5,29,by_fx||!mix?SP_MUT:SP_GREEN);
    sp_text(25,32,&SP_F17,PG_NAME[pg],SP_INK,SP_PAPER[0]);
    sp_right(219,40,&SP_F7,state,by_fx||!mix?SP_MUT:SP_GREEN,SP_PAPER[0]);
}
static void sx_screw(int32_t x,int32_t y)
{disc(x,y,2,SP_LINE);disc(x,y,1,SP_PANEL);cv_line(x-1,y,x+1,y,SP_MUT);}
static void sx_oval(int32_t x,int32_t y,int32_t rx,int32_t ry,uint16_t c)
{
    uint32_t k;int32_t px=x+rx,py=y;
    for(k=1;k<=32u;k++){
        uint32_t a=k*(1u<<27);int32_t xx=x+cs(a,rx),yy=y+sn(a,ry);
        cv_line(px,py,xx,yy,c);px=xx;py=yy;
    }
}
static void sx_plate(int32_t bottom)
{
    int32_t x,y;rrect(18,68,204,bottom-67,5,SP_LINE);rrect(19,69,202,bottom-69,4,SL_PLATE);
    for(x=24;x<=216;x+=192)for(y=74;y<=bottom-6;y+=bottom-80)sx_screw(x,y);
}
static void sx_vibrato(int32_t mix)
{
    int32_t x,y,py=0,depth=PV(P_VDEPTH),mod=p_mod/3;
    uint32_t inc=MEL_LFO[PV(P_VRATE)]*300u;
    uint16_t color=mix565(SP_LINE,SP_GREEN,(uint32_t)(by_fx?0:mix)*16u/100u);
    if(mod>depth)depth=mod;depth=by_fx||!mix?0:depth*21/100;
    sx_plate(139);
    for(y=84;y<=122;y+=19)cv_rect(34,y,173,1,(smktron_theme?RGB(47,65,54):RGB(181,200,190)));
    for(x=62;x<=178;x+=29)cv_rect(x,78,1,50,(smktron_theme?RGB(47,65,54):RGB(181,200,190)));
    for(x=0;x<=172;x++){
        y=103-sn(fxv_vib-(uint32_t)x*inc,depth);
        if(x)thick(33+x,py,34+x,y,2,color);py=y;
    }
    disc(120,103-sn(fxv_vib-86u*inc,depth),3,by_fx||!mix?SP_MUT:SP_INK);
    sp_text(76,140,&SP_F7,"PITCH MODULATION",SP_MUT,SP_BG);
}
static void sx_leslie(int32_t mix)
{
    int32_t x,k,mx=46+PV(P_LSPREAD)*20/100;
    uint16_t col=by_fx||!mix?SP_MUT:SP_GREEN;
    rrect(72,65,96,80,4,SP_LINE);rrect(73,66,94,78,3,(smktron_theme?RGB(32,47,37):RGB(179,199,187)));
    rrect(80,72,80,37,2,(smktron_theme?RGB(23,34,28):RGB(220,229,221)));rrect(80,114,80,23,2,(smktron_theme?RGB(23,34,28):RGB(220,229,221)));
    sx_oval(120,91,31,8,SP_LINE);
    thick(120-cs(fxv_horn,29),91-sn(fxv_horn,7),120+cs(fxv_horn,29),91+sn(fxv_horn,7),3,SP_INK);
    disc(120,91,4,col);ellipse(120+cs(fxv_horn,29),91+sn(fxv_horn,7),5,4,col);
    ellipse(120,125,18,8,SP_REEL[0]);ellipse(120+cs(fxv_drum,13),125+sn(fxv_drum,5),3,2,SP_INK);
    for(k=-1;k<=1;k+=2){x=120+k*mx;disc(x,94,3,SP_INK);cv_rect(x,97,1,27,SP_MUT);cv_rect(x-6,125,13,1,SP_MUT);}
    sp_text(44,73,&SP_F7,"L",SP_MUT,SP_BG);sp_text(192,73,&SP_F7,"R",SP_MUT,SP_BG);
    sp_text(101,138,&SP_F7,N_SPEED[PV(P_LSPEED)],SP_MUT,SP_BG);
}
/* Capsule perimeter at constant speed, one lap per current echo TIME.
 * The dot is a timing diagram; it is not a tape head position in the DSP. */
static void sx_echo(int32_t mix,uint32_t energy)
{
    int32_t k,h=13,ms=echo_ms(PV(P_ETIME));
    uint32_t u=(fm1_ms%(uint32_t)ms)*340u/(uint32_t)ms;
    sx_plate(143);rrect(43,79,154,39,19,SP_TAPE);rrect(47,83,146,31,15,SP_BG);
    if(mix&&energy>1u){
        int32_t x,y;
        if(u<116u){x=62+(int32_t)u;y=81;}
        else if(u<170u){uint32_t a=0xC0000000u+(u-116u)*(0x80000000u/54u);x=178+cs(a,17);y=98+sn(a,17);}
        else if(u<286u){x=178-(int32_t)(u-170u);y=115;}
        else {uint32_t a=0x40000000u+(u-286u)*(0x80000000u/54u);x=62+cs(a,17);y=98+sn(a,17);}
        disc(x,y,3,SP_GREEN);
    }
    for(k=0;k<3;k++){rrect(84+k*31,109,11,12,1,SP_MUT);cv_rect(89+k*31,110,1,7,SP_INK);}
    for(k=0;k<7;k++){
        uint16_t c=mix&&energy>1u?mix565(SP_LINE,mix565(SP_GREEN,SP_INK,(uint32_t)(100-PV(P_ETONE))*8u/100u),(uint32_t)(h+3)*(uint32_t)mix/100u):SP_LINE;
        if(h>0)rrect(57+k*20,138-h,5,h,1,c);
        h=h*(20+PV(P_EFB)*75/100)/100;
    }
}
static void sx_reverb(int32_t mix,uint32_t energy)
{
    int32_t j,r,pre=PV(P_RPRE)*22/100,size=18+PV(P_RSIZE)*29/100;
    rrect(29,69,182,72,4,SP_LINE);rrect(30,70,180,70,3,(smktron_theme?RGB(30,43,34):RGB(202,215,206)));
    cv_line(29,69,67,81,SP_LINE);cv_line(211,69,173,81,SP_LINE);
    cv_line(29,140,67,126,SP_LINE);cv_line(211,140,173,126,SP_LINE);
    rrect(67,81,106,46,1,SP_LINE);rrect(68,82,104,44,1,(smktron_theme?RGB(23,34,28):RGB(220,229,221)));
    for(j=0;j<4;j++){
        uint32_t ph=(fm1_ms/(uint32_t)(3+(100-PV(P_RSIZE))/12)+(uint32_t)j*64u)&255u;
        uint16_t c=SP_LINE;
        r=7+(int32_t)ph*size/256;
        if(mix&&energy>1u)c=mix565((smktron_theme?RGB(23,34,28):RGB(220,229,221)),mix565(SP_GREEN,SP_INK,(uint32_t)PV(P_RTONE)*8u/100u),(16u-ph/16u)*(uint32_t)mix/100u);
        sx_oval(120,103,r,r*43/100,c);
    }
    rrect(114,96,13,15,2,SP_INK);disc(120,103,3,SP_PAPER[0]);
    cv_rect(37,132,4+pre,2,by_fx?SP_MUT:SP_GREEN);sp_text(37,118,&SP_F7,"PRE",SP_MUT,(smktron_theme?RGB(30,43,34):RGB(202,215,206)));
}
static void sx_status(uint32_t sound,uint32_t pg,int32_t mix,uint32_t energy)
{
    const char *state=by_fx?(energy>1u&&(pg==PG_ECHO||pg==PG_VERB)?"FX OUT / TRAILS":"FX OUT"):!mix?"FX OFF":
        pg==PG_VIB?(p_mod*100/127>PV(P_VMIX)?"MOD WHEEL":mix<70?"CHORUS":"VIBRATO"):pg==PG_LES?N_SPEED[PV(P_LSPEED)]:"FX IN";
    cv_begin(240,18,SP_BG);cv_oy=-148;disc(14,155,2,by_fx||!mix?SP_LINE:SP_GREEN);
    sp_text(21,150,&SP_F7,state,SP_MUT,SP_BG);
    sp_right(228,150,&SP_F7,MEL_SET_NAME[sound],SP_ACCENT[sound],SP_BG);
    cv_rect(10,164,221,2,SP_INK);cv_oy=0;cv_blit(0,148);
}
static void smktron_fx_draw(uint32_t base)
{
    uint32_t k,sound=(uint32_t)PV(P_SOUND)%4u,sig=2166136261u,energy=sx_energy(page);
    uint32_t phase=page==PG_VIB?fxv_vib:page==PG_LES?fxv_horn^fxv_drum:fm1_ms;
    int32_t mix=sx_mix(page);int full=!sx_on||sx_page!=page||redraw==0xFFu;
    int moving=page==PG_LES||(page==PG_VIB&&!by_fx&&mix)||(energy>1u);
    for(k=0;k<4u;k++)sig=(sig^(uint32_t)PV(CTL[page][k].pid))*16777619u;
    sig=(sig^sound)*16777619u;sig=(sig^by_fx)*16777619u;
    if(page==PG_VIB)sig=(sig^(uint32_t)p_mod)*16777619u;
    sig=(sig^(energy>1u))*16777619u;
    if(full||(redraw&1u))sp_header_title(sound,"EFFECTS");
    if(full||sig!=sx_sig||(moving&&phase!=sx_phase&&fm1_ms-sx_time>=50u)){
        cv_begin(240,124,SP_BG);cv_oy=-24;sx_label(page,mix);
        if(page==PG_VIB)sx_vibrato(mix);else if(page==PG_LES)sx_leslie(mix);
        else if(page==PG_ECHO)sx_echo(mix,energy);else sx_reverb(mix,energy);
        cv_oy=0;cv_blit(0,24);sx_time=fm1_ms;sx_phase=phase;
    }
    if(full||sig!=sx_sig)sx_status(sound,page,mix,energy);
    sp_keyboard(base,full||(redraw&4u));
    if(full||sig!=sx_sig||(redraw&8u))sp_parameters_for(page);
    sx_on=1;sx_page=page;sx_sig=sig;
}
static void smktron_fx_leave(void){sx_on=0;}
