/* SPDX-License-Identifier: GPL-3.0-only
 * SMKTRON PLAY UI. Included by mel_ui.c, SMK-37 only.
 * The renderer only reads engine state. No changes to audio, parameters, clock,
 * voice allocation or storage. Canvas tiles reuse the existing 240x124 pool.
 */
#include "smktron_fonts.h"
#define SP_BG (smktron_theme?RGB(19,26,25):RGB(228,232,229))
#define SP_PANEL (smktron_theme?RGB(30,39,37):RGB(200,208,204))
#define SP_INK (smktron_theme?RGB(191,207,198):RGB(36,49,46))
#define SP_MUT (smktron_theme?RGB(131,154,143):RGB(96,112,106))
#define SP_LINE (smktron_theme?RGB(55,74,66):RGB(163,178,171))
#define SP_GREEN (smktron_theme?RGB(82,165,143):RGB(32,124,112))
#define SP_RIM (smktron_theme?RGB(91,113,101):RGB(220,226,220))
#define SP_TAPE (smktron_theme?RGB(125,99,67):RGB(93,70,52))
#define SP_WHITE (smktron_theme?RGB(119,133,124):RGB(243,241,232))
#define SP_BLACK (smktron_theme?RGB(10,16,14):RGB(23,32,31))
static const uint16_t SP_PAPER_THEMES[2][4]={{RGB(246,241,223),RGB(231,223,237),RGB(240,224,200),RGB(223,235,239)},{RGB(39,57,48),RGB(49,40,60),RGB(62,47,30),RGB(32,50,62)}};
#define SP_PAPER (SP_PAPER_THEMES[smktron_theme])
static const uint16_t SP_REEL_THEMES[2][4]={{RGB(138,174,163),RGB(165,151,185),RGB(195,160,113),RGB(135,172,189)},{RGB(62,106,91),RGB(95,76,116),RGB(132,100,58),RGB(61,99,119)}};
#define SP_REEL (SP_REEL_THEMES[smktron_theme])
static const uint16_t SP_ACCENT_THEMES[2][4]={{RGB(32,124,112),RGB(118,96,131),RGB(146,107,54),RGB(70,111,134)},{RGB(82,165,143),RGB(156,126,179),RGB(186,143,79),RGB(110,157,185)}};
#define SP_ACCENT (SP_ACCENT_THEMES[smktron_theme])

static int32_t sp_width(const sp_font_t *f,const char *s)
{
    int32_t w=0;
    for(;*s;s++) { uint32_t ch=(uint8_t)*s;if(ch>f->last&&ch>='a'&&ch<='z')ch-=32u;if(ch<32u||ch>f->last)ch='?';w+=f->w[ch-32u]; }
    return w;
}
static void sp_text(int32_t x,int32_t y,const sp_font_t *f,const char *s,uint16_t fg,uint16_t bg)
{
    uint16_t col[4]={bg,mix565(bg,fg,5),mix565(bg,fg,11),fg};
    for(;*s;s++) {
        uint32_t ch=(uint8_t)*s,w,at,xx,yy;
        const uint8_t *d;
        if(ch>f->last&&ch>='a'&&ch<='z')ch-=32u;
        if(ch<32u||ch>f->last)ch='?';
        ch-=32u;w=f->w[ch];d=f->data+f->off[ch];
        for(yy=0,at=0;yy<f->h;yy++)for(xx=0;xx<w;xx++,at++) {
            uint32_t a=(d[at/4u]>>(6u-2u*(at%4u)))&3u;
            if(a)cv_pset(x+(int32_t)xx,y+(int32_t)yy,col[a]);
        }
        x+=(int32_t)w;
    }
}
static void sp_right(int32_t x,int32_t y,const sp_font_t *f,const char *s,uint16_t fg,uint16_t bg)
{ sp_text(x-sp_width(f,s),y,f,s,fg,bg); }
/* A clipped convex quad is sufficient for each of the three rotating cutouts. */
static void sp_quad(const int32_t *x,const int32_t *y,uint16_t col)
{
    int32_t lo=y[0],hi=y[0],yy,k;
    for(k=1;k<4;k++){if(y[k]<lo)lo=y[k];if(y[k]>hi)hi=y[k];}
    for(yy=lo;yy<=hi;yy++){
        int32_t xmin=999,xmax=-999;
        for(k=0;k<4;k++){
            int32_t j=(k+1)%4,ya=y[k],yb=y[j],v;
            if(ya==yb)continue;
            if(yy<(ya<yb?ya:yb)||yy>=(ya>yb?ya:yb))continue;
            v=x[k]+(yy-ya)*(x[j]-x[k])/(yb-ya);
            if(v<xmin)xmin=v;if(v>xmax)xmax=v;
        }
        if(xmin<=xmax)cv_rect(xmin,yy,xmax-xmin+1,1,col);
    }
}
static void sp_reel_sized(int32_t x,int32_t y,uint32_t phase,uint32_t sound,int32_t radius,int enabled)
{
    uint32_t k;
    int32_t rim=(30*radius+17)/34,outer=(26*radius+17)/34,inner=(11*radius+17)/34;
    int32_t hub=(9*radius+17)/34,cap=(4*radius+17)/34;
    disc(x,y,radius+1,SP_LINE);disc(x,y,radius,SP_RIM);
    disc(x,y,rim,enabled?SP_TAPE:SP_LINE);disc(x,y,outer,enabled?SP_REEL[sound]:SP_PANEL);
    for(k=0;k<3u;k++){
        uint32_t a=phase+k*0x55555555u;
        int32_t xx[4]={x+cs(a-0x08800000u,inner),x+cs(a-0x0C400000u,outer),x+cs(a+0x0C400000u,outer),x+cs(a+0x08800000u,inner)};
        int32_t yy[4]={y+sn(a-0x08800000u,inner),y+sn(a-0x0C400000u,outer),y+sn(a+0x0C400000u,outer),y+sn(a+0x08800000u,inner)};
        sp_quad(xx,yy,SP_BG);
    }
    disc(x,y,hub,enabled?SP_INK:SP_MUT);disc(x,y,cap,enabled?SP_GREEN:SP_LINE);cv_line(x-2,y,x+2,y,SP_BG);
}
static void sp_reel(int32_t x,int32_t y,uint32_t phase,uint32_t sound)
{ sp_reel_sized(x,y,phase,sound,34,1); }
static void sp_header_title(uint32_t sound,const char *title)
{
    char tape[12],shortmsg[32];uint32_t i;
    int volume=page==PG_PLAY||page==PG_LAYER;
    cv_begin(240,24,SP_BG);cv_oy=0;
    sp_text(12,6,&SP_F10,"SMKTRON",SP_INK,SP_BG);
#if SMKTRON_REFINEMENTS
    {
        uint16_t color=smk_battery_low?RGB(178,52,42):SP_MUT;
        cv_rect(89,7,16,9,color);cv_rect(105,10,2,3,color);
        cv_rect(91,9,12,5,SP_BG);
        for(i=0;i<smk_battery_level;i++)cv_rect(91+(int32_t)i*3,9,2,5,color);
    }
#endif
    if(volume){
        put(put_int(put(tape,"VOL "),(smk_master*100+511)/1023,0),"%");
        sp_text(136-sp_width(&SP_F7,tape)/2,8,&SP_F7,tape,SP_INK,SP_BG);
    }
    if(t_msg){
        for(i=0;i<31u&&msg[i];i++)shortmsg[i]=msg[i];shortmsg[i]=0;
        while(i&&sp_width(&SP_F7,shortmsg)>
#if SMKTRON_REFINEMENTS
            (volume?64:108)
#else
            132
#endif
        )shortmsg[--i]=0;
        sp_right(228,8,&SP_F7,shortmsg,SP_GREEN,SP_BG);
    }
#if SMKTRON_SCENES
    else if(sc_on){
        put(tape,sc_pin?"PRESETS ":"SCENES ");
        if(sc_selected<8u)put_int(put(tape,sc_pin?"PRESETS ":"SCENES "),(int32_t)sc_selected+1,0);
        sp_right(228,7,&SP_F8,tape,SP_GREEN,SP_BG);
    }
#endif
    else if(title){
        sp_right(228,7,&SP_F8,title,SP_MUT,SP_BG);
    }else{
        put_int(put(tape,"TAPE 0"),(int32_t)sound+1,0);
        sp_right(228,7,&SP_F8,tape,SP_MUT,SP_BG);
    }
    cv_rect(12,22,216,1,SP_LINE);cv_blit(0,0);
}
static void sp_header(uint32_t sound){sp_header_title(sound,0);}
static void sp_stage(uint32_t sound)
{
    char bank[2]={(char)('A'+sound),0};
    cv_begin(240,124,SP_BG);cv_oy=-24;
    rrect(12,29,217,29,2,SP_LINE);rrect(13,30,215,27,1,SP_PAPER[sound]);
    cv_rect(12,29,5,29,SP_ACCENT[sound]);
    sp_text(25,32,&SP_F17,MEL_SET_NAME[sound],SP_INK,SP_PAPER[sound]);
    sp_right(220,39,&SP_F10,bank,SP_GREEN,SP_PAPER[sound]);
    thick(62,106,82,142,3,SP_TAPE);thick(82,142,157,142,3,SP_TAPE);thick(157,142,178,106,3,SP_TAPE);
    sp_reel(62,97,reel_a,sound);sp_reel(178,97,reel_a,sound);
    disc(83,141,4,SP_LINE);disc(83,141,3,SP_RIM);disc(157,141,4,SP_LINE);disc(157,141,3,SP_RIM);
    rrect(106,134,29,14,2,SP_INK);cv_rect(111,140,19,2,SP_GREEN);
    cv_oy=0;cv_blit(0,24);
}
static void sp_badge(int32_t *x,const char *s,uint16_t color)
{
    int32_t w=sp_width(&SP_F7,s);
    if(*x-w<55)return;
    *x-=w;sp_text(*x,150,&SP_F7,s,color,SP_BG);*x-=7;
}
static void sp_status(void)
{
    int32_t x=228;char layer[24];
    cv_begin(240,18,SP_BG);cv_oy=-148;
    disc(14,156,2,ui_lv?SP_GREEN:SP_LINE);
    sp_text(21,150,&SP_F7,ui_lv?"PLAY":"READY",SP_GREEN,SP_BG);
    /* Prioritise performance state and layer; omit lower-priority effect badges if full. */
    if(PV(P_BSND)){put(put(layer,"LYR "),MEL_SET_NAME[(uint32_t)PV(P_BSND)-1u]);sp_badge(&x,layer,SP_INK);}
    if(seq_rec)sp_badge(&x,"REC",C_RED);
    if(seq_play)sp_badge(&x,"SEQ",SP_INK);
    if(arp_on)sp_badge(&x,"ARP",SP_INK);
    if(hold)sp_badge(&x,"HOLD",SP_INK);
    if(set.half)sp_badge(&x,"1/2",SP_INK);
    if(!by_fx&&PV(P_RMIX))sp_badge(&x,"VERB",SP_MUT);
    if(!by_fx&&PV(P_EMIX))sp_badge(&x,"ECHO",SP_MUT);
    if(!by_lofi&&(PV(P_WOW)||PV(P_FLUT)))sp_badge(&x,"WOW",SP_MUT);
    cv_rect(10,164,221,2,SP_INK);
    cv_oy=0;cv_blit(0,148);
}
static uint32_t sp_key_sig;
static void sp_keyboard(uint32_t base,int force)
{
    uint8_t lit[37];uint32_t k,v,sig=2166136261u;int32_t wx[37],white=0;
    base-=5u;
    for(k=0;k<37u;k++){
        uint32_t note=base+k,a=0,t=note<128u?tpos[note]:0u;
        for(v=0;v<NV;v++)if(voice[v].busy&&voice[v].note==note&&(uint32_t)voice[v].amp>a)a=(uint32_t)voice[v].amp;
        lit[k]=(uint8_t)(!a?0u:PV(P_LEN)?1u+(TAPE_LEN-(t>TAPE_LEN?TAPE_LEN:t))*23u/TAPE_LEN:24u);
        sig=(sig^lit[k])*16777619u;
        /* Fixed physical C..C geometry, even when MIDI pitch/octave is transposed. */
        wx[k]=12+white*216/22;
        if(!black(48u+k))white++;
    }
    if(!force&&sig==sp_key_sig)return;sp_key_sig=sig;
    cv_begin(240,34,SP_BG);cv_oy=-166;
    rrect(10,164,221,30,2,SP_INK);
    for(k=0;k<37u;k++)if(!black(48u+k)){
        int32_t w=9,x=wx[k],h=(int32_t)lit[k]*26/24;
        rrect(x,166,w,27,1,SP_WHITE);
        if(h)rrect(x,193-h,w,h,1,smktron_theme?RGB(255,144,32):SP_GREEN);
    }
    for(k=0;k<37u;k++)if(black(48u+k)){
        int32_t x=wx[k]-3,h=(int32_t)lit[k]*17/24;
        rrect(x,166,6,17,1,SP_BLACK);
        if(h)rrect(x,183-h,6,h,1,smktron_theme?RGB(255,144,32):SP_GREEN);
    }
    cv_oy=0;cv_blit(0,166);
}
static void sp_parameters_for(uint32_t pg)
{
    uint32_t k;
    cv_begin(240,40,SP_PANEL);cv_oy=-200;
    for(k=0;k<4u;k++){
        const ctl_t *ctl=&CTL[pg][k];char val[20];
        int inactive=(pg==PG_LAYER&&!PV(P_BSND)&&k>0u)||((pg==PG_TAPE||pg==PG_WEAR)&&by_lofi)||(pg>=PG_VIB&&pg<=PG_VERB&&by_fx);
        int32_t value=pg==PG_SEQ&&k==2u?(int32_t)seq_cur:PV(ctl->pid);
        int32_t x=8+(int32_t)k*59,at=pg==PG_SEQ&&k==2u?value*46/15:pos256(ctl->pid)*46/256,zero=0;
        uint16_t bg=SP_PANEL;
        if(touched==k){bg=(smktron_theme?RGB(39,61,49):RGB(181,205,192));cv_rect(x-3,202,55,32,bg);}
        if(k)cv_rect(x-4,207,1,26,SP_LINE);
        sp_text(x,203,&SP_F7,ctl->name,SP_MUT,bg);
        fmt_val(ctl,value,val);
        sp_text(x,215,sp_width(&SP_F14,val)>51?&SP_F10:&SP_F14,val,inactive?SP_MUT:SP_INK,bg);
        cv_rect(x,235,47,2,SP_LINE);
        if(PINFO[ctl->pid][0]<0&&PINFO[ctl->pid][1]>0)zero=(-PINFO[ctl->pid][0])*46/(PINFO[ctl->pid][1]-PINFO[ctl->pid][0]);
        if(!inactive){if(at>=zero)cv_rect(x+zero,235,at-zero+1,2,SP_GREEN);else cv_rect(x+at,235,zero-at+1,2,SP_GREEN);}
    }
    cv_oy=0;cv_blit(0,200);
}
static void sp_parameters(void){sp_parameters_for(PG_PLAY);}
static uint32_t sp_on,sp_last_sound=99,sp_angle,sp_time,sp_status_sig;
static void smktron_play_draw(uint32_t base)
{
    uint32_t sound=(uint32_t)PV(P_SOUND)%4u;
    int full=!sp_on||sp_last_sound!=sound||redraw==0xFFu;
    uint32_t sig=(ui_lv>0)|((uint32_t)PV(P_BSND)<<1)|(hold<<5)|(set.half<<6)|(seq_rec<<7)|(seq_play<<8)|(arp_on<<9)|((uint32_t)by_fx<<10)|((uint32_t)by_lofi<<11)|((PV(P_RMIX)>0)<<12)|((PV(P_EMIX)>0)<<13)|(((PV(P_WOW)||PV(P_FLUT))?1u:0u)<<14);
    if(full||(redraw&1u))sp_header(sound);
    if(full||(((redraw&2u)||sp_angle!=reel_a)&&fm1_ms-sp_time>=50u)){sp_stage(sound);sp_angle=reel_a;sp_time=fm1_ms;}
    if(full||sig!=sp_status_sig){sp_status();sp_status_sig=sig;}
    sp_keyboard(base,full||(redraw&4u));
    if(full||(redraw&8u))sp_parameters();
    sp_on=1;sp_last_sound=sound;
}
static void smktron_play_leave(void){sp_on=0;}
