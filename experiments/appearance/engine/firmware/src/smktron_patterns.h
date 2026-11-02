/* SPDX-License-Identifier: GPL-3.0-only
 * SMKTRON ARP/SEQ. Presentation only, no sequencer or clock writes.
 */
static void sq_note(char *s,uint32_t n)
{
    static const char *const names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    put_int(put(s,names[n%12u]),(int32_t)(n/12u)-1,0);
}
static void sq_label(void)
{
    const char *state=page==PG_ARP?(arp_on?"ON":"OFF"):seq_rec?"REC":seq_play?"PLAY":"STOP";
    rrect(12,29,217,29,2,SP_LINE);rrect(13,30,215,27,1,SP_PAPER[0]);
    cv_rect(12,29,5,29,SP_GREEN);sp_text(25,32,&SP_F17,PG_NAME[page],SP_INK,SP_PAPER[0]);
    sp_right(219,40,&SP_F7,state,seq_rec&&page==PG_SEQ?C_RED:SP_GREEN,SP_PAPER[0]);
}
static void sq_arp(void)
{
    uint32_t i,n=arp_n,lo=127u;char note[12],count[12];
    rrect(18,67,204,78,4,SP_LINE);rrect(19,68,202,76,3,SL_PLATE);
    sp_text(27,72,&SP_F7,ARP_MODE[PV(P_AMODE)],SP_MUT,SL_PLATE);
    put(put_int(count,(int32_t)n,0)," NOTES");sp_right(213,72,&SP_F7,count,SP_MUT,SL_PLATE);
    if(!n){const char *s=arp_on?"PLAY A CHORD":"PRESS ARP TO START";
        sp_text(120-sp_width(&SP_F8,s)/2,103,&SP_F8,s,SP_MUT,SL_PLATE);return;}
    for(i=0;i<n;i++)if(arp_list[i]<lo)lo=arp_list[i];
    /* All 48 entries remain visible; no truncation at two octaves. */
    for(i=0;i<n;i++){
        int32_t x=27+(int32_t)(i*186u/n),end=27+(int32_t)((i+1u)*186u/n);
        int32_t h=8+(int32_t)(arp_list[i]-lo),w=end-x-1;
        int active=arp_on&&i==arp_pos_shown&&arp_cur==arp_list[i];
        if(h>38)h=38;if(w<1)w=1;
        rrect(x,131-h,w,h,1,active?SP_GREEN:SP_REEL[0]);
        if(i==arp_pos_shown)cv_rect(x,134,w,2,SP_INK);
    }
    if(arp_cur<128u){sq_note(note,arp_cur);sp_text(120-sp_width(&SP_F7,note)/2,82,&SP_F7,note,SP_INK,SL_PLATE);}
}
static void sq_seq(void)
{
    uint32_t i,k,len=(uint32_t)PV(P_SLEN);
    for(i=0;i<16u;i++){
        int32_t x=20+(int32_t)(i%8u)*25,y=67+(int32_t)(i/8u)*36;
        int enabled=i<len,playing=seq_play&&i==seq_step,cursor=i==seq_cur;
        uint16_t bg=!enabled?SP_BG:playing?SP_GREEN:SL_PLATE;char number[4];
        rrect(x,y,23,31,2,enabled?SP_LINE:SP_PANEL);rrect(x+1,y+1,21,29,1,bg);
        number[0]=(char)('0'+(i+1u)/10u);number[1]=(char)('0'+(i+1u)%10u);number[2]=0;
        sp_text(x+3,y+2,&SP_F7,number,playing?SP_PAPER[0]:enabled?SP_MUT:SP_LINE,bg);
        for(k=0;k<4u;k++)if(set.steps[i][k]){
            int32_t nt=set.steps[i][k],ny=y+25-(nt-36)*13/60;
            if(ny<y+13)ny=y+13;if(ny>y+26)ny=y+26;
            cv_rect(x+3+(int32_t)k*4,ny,3,3,!enabled?SP_LINE:playing?SP_PAPER[0]:SP_GREEN);
        }
        if(cursor)cv_rect(x,y+32,23,2,seq_rec?C_RED:SP_INK);
    }
    sp_text(20,140,&SP_F7,seq_rec?(seq_play?"LIVE REC":"STEP REC"):seq_play?"PLAYING":"STOPPED",seq_rec?C_RED:SP_MUT,SP_BG);
}
static void sq_status(uint32_t sound)
{
    char status[28],*p;
    cv_begin(240,18,SP_BG);cv_oy=-148;
    if(page==PG_SEQ){p=put_int(put(status,"EDIT "),(int32_t)seq_cur+1,0);
        if(seq_play)put_int(put(p," / PLAY "),(int32_t)seq_step+1,0);}
    else put(status,arp_on?(arp_cur<128u?"ARP RUN":"ARP READY"):"ARP OFF");
    disc(14,155,2,seq_rec&&page==PG_SEQ?C_RED:SP_GREEN);sp_text(21,150,&SP_F7,status,SP_MUT,SP_BG);
    sp_right(228,150,&SP_F7,MEL_SET_NAME[sound],SP_ACCENT[sound],SP_BG);
    cv_rect(10,164,221,2,SP_INK);cv_oy=0;cv_blit(0,148);
}
static uint32_t sq_on,sq_page,sq_sig,sq_time;
static void smktron_patterns_draw(uint32_t base)
{
    uint32_t k,sound=(uint32_t)PV(P_SOUND)%4u,sig=2166136261u;
    int full=!sq_on||sq_page!=page||redraw==0xFFu;
    for(k=0;k<4u;k++)sig=(sig^(uint32_t)PV(CTL[page][k].pid))*16777619u;
    sig=(sig^sound)*16777619u;sig=(sig^PV(P_LOOK))*16777619u;sig=(sig^touched)*16777619u;
    if(page==PG_ARP){
        sig=(sig^arp_on)*16777619u;sig=(sig^arp_cur)*16777619u;sig=(sig^arp_pos_shown)*16777619u;
        for(k=0;k<arp_n;k++)sig=(sig^arp_list[k])*16777619u;
        sig=(sig^arp_n)*16777619u;
    }else{
        sig=(sig^seq_play)*16777619u;sig=(sig^seq_rec)*16777619u;
        sig=(sig^seq_step)*16777619u;sig=(sig^seq_cur)*16777619u;
        for(k=0;k<64u;k++)sig=(sig^set.steps[k/4u][k%4u])*16777619u;
    }
    if(full||(redraw&1u))sp_header_title(sound,"PATTERN");
    if(full||sig!=sq_sig||(PV(P_LOOK)&&ui_lv&&fm1_ms-sq_time>=50u)){
        cv_begin(240,124,SP_BG);cv_oy=-24;sq_label();if(page==PG_ARP)sq_arp();else sq_seq();
        cv_oy=0;tape_look(124);cv_blit(0,24);sq_time=fm1_ms;
    }
    if(full||sig!=sq_sig)sq_status(sound);
    sp_keyboard(base,full||(redraw&4u));
    if(full||sig!=sq_sig||(redraw&8u))sp_parameters_for(page);
    sq_on=1;sq_page=page;sq_sig=sig;
}
static void smktron_patterns_leave(void){sq_on=0;}
