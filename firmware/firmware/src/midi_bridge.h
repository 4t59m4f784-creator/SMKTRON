/* SPDX-License-Identifier: GPL-3.0-only. SMKTRON integration, Marcus Padrini / ChatGPT. */
static int clampi(int x,int lo,int hi){return x<lo?lo:x>hi?hi:x;}
static unsigned slen(const char *s){unsigned n=0;while(s[n])n++;return n;}
static void scat(char *s,const char *v){put(s+slen(s),v);}
static void fmt_u(char *s,unsigned v){put_int(s,(int32_t)v,0);}
static void fmt_s(char *s,int v){put_int(s,v,0);}
#include "midi_storage.h"
#include "smk_midi_mode.h"
static unsigned mm_exiting,mm_return_page,mm_return_ch,mm_return_sc,mm_return_pin;
static struct {uint32_t since,ready,rest_high,candidate,down,blocked;} mm_pedal;
static void mm_sustain_poll(void){
 unsigned raw=fm1_in.pedal,high;if(fm1_in.frames<16u)return;
 if(!mm_pedal.ready){high=raw>=512u;if(!mm_pedal.since||high!=mm_pedal.rest_high){mm_pedal.rest_high=high;mm_pedal.since=fm1_ms+1u;}if(fm1_ms+1u-mm_pedal.since>=100u)mm_pedal.ready=1;return;}
 high=mm_pedal.rest_high?raw<256u:raw>768u;
 if(mm_pedal.down)high=mm_pedal.rest_high?raw<768u:raw>256u;
 if(high!=mm_pedal.candidate){mm_pedal.candidate=high;mm_pedal.since=fm1_ms;}
 if(fm1_ms-mm_pedal.since<12u)return;
 if(mm_pedal.blocked){if(!high)mm_pedal.blocked=0;return;}
 if(high==mm_pedal.down)return;mm_pedal.down=high;if(mm_on&&!mm_exiting)mm_pedal_event(high);
}
static void mm_discard_midi(void){uint8_t s,a,b;while(midi_get(&s,&a,&b)){} }
static unsigned mm_any_pad(void){for(unsigned p=0;p<16;p++)if(fm1_in.pad_st[p])return 1;return 0;}
static void mm_drain_panel(void){while(smk_extkey_take()>=0){}while(smk_pad_take()>=0){}for(unsigned e=0;e<8;e++)fm1_enc_take(e);uint32_t r;fm1_input_edges(&r);}
static void mm_clear_audio(void){
 mm_fx_reset=1;
 fm1_irq_off();memset(voice,0,sizeof voice);memset(gr_voice,0,sizeof gr_voice);memset((void*)tpos,0,sizeof tpos);fm1_irq_on();
 /* app_audio is gated while the tank is cleared; keep input/USB timer alive. */
 memset(vib_buf,0,sizeof vib_buf);memset(les_hb,0,sizeof les_hb);memset(les_db,0,sizeof les_db);memset(echo_buf,0,sizeof echo_buf);memset(ovp_fx_mem,0,sizeof ovp_fx_mem);
 ovp_rv_lp_in=ovp_rv_lpl=ovp_rv_lpr=ovp_rv_yl=ovp_rv_yr=0;
}
static void mm_enter(void){
 if(mm_on)return;mm_return_page=page;mm_return_ch=ch_on;mm_return_sc=sc_on;mm_return_pin=sc_pin;
 midi_wire_enter();mm_on=1;seq_start(0);seq_rec=0;ch_stop();ch_rec=ch_hold=0;ch_target=255u;ch_capture_reset();hold=pedal=0;arp_release();
 for(unsigned k=0;k<128;k++){src_set(k,0,255u);midi_note[k]=0;}
 memset(smk_ext_note,0,sizeof smk_ext_note);for(unsigned k=0;k<27;k++)held_note[k]=255;
 mm_clear_audio();scene_pending=255u;scene_gain=scene_target=32768u;gr_focus=0;sc_save_down=0;save_req=0;
 mm_drain_panel();mm_edit=mm_settings=mm_pick=mm_reset_ask=mm_exiting=0;mm_oct=mm_kbank=mm_fbank=mm_pbank=0;
 mm_pedal.blocked=mm_pedal.down||mm_pedal.candidate;mm_pedal.down=0;mm_safety();mm_discard_midi();mm_status="READY";boot_done=1;mm_lcd_live=1;
}
static void mm_leave(void){mm_lcd_live=0;mm_safety();midi_wire_begin_leave();mm_discard_midi();mm_exiting=1;mm_edit=mm_settings=mm_pick=mm_reset_ask=0;mm_pedal.blocked=mm_pedal.down||mm_pedal.candidate;mm_pedal.down=0;mm_status="RETURNING TO TAPE";}
static void mm_finish_leave(void){
 mm_exiting=0;mm_wait_release=1;page=PG_MIDI;ch_on=mm_return_ch;sc_on=mm_return_sc;sc_pin=mm_return_pin;
 mm_drain_panel();prev_keys=fm1_in.notes;memset(smk_ext_note,0,sizeof smk_ext_note);p_bend16=0;p_mod=0;apply();
 mm_wheel_sync=1;mm_on=0;redraw=255u;lcd_fill(0,0,240,240,SP_BG);say("SMKTRON");
}
static void mm_entry_draw(void);
#include "smk_midi_mode_ui.h"
