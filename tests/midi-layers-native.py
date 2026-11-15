#!/usr/bin/env python3
"""Run SMKSYNTH's ML1 ownership/backpressure suite against the SMKTRON core.
UI entry/exit are covered separately by the full WASM engine suite.
"""
from pathlib import Path
import tempfile,subprocess
ROOT=Path(__file__).resolve().parents[1];SRC=ROOT/'firmware/firmware/src'
def fn(s,sig):
 a=s.index(sig);b=s.index('{',a);i=b+1;n=1
 while n:n+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
ui=(SRC/'smk_midi_mode_ui.h').read_text()
# Identical assertions to the upstream SMKSYNTH test, with a minimal panel HAL.
case=(ROOT/'tests/fixtures/smksynth-ml1-cases.c').read_text()
prefix=r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
static unsigned mm_on,fm1_ms,host_midi_count,host_midi_blocked,smktron_pad_scale=4;
static struct{uint64_t keys;uint8_t key_vel[37],pad_st[16],fader[4];uint16_t wheel[2];uint32_t frames,buttons;}fm1_in;
static uint8_t host_midi_sent[256][3];static unsigned padled_rgb[16];
enum{BTN_KNOB_BANK,BTN_FADER_BANK,BTN_LEFT,BTN_RIGHT,BTN_OCT_DOWN,BTN_OCT_UP,BTN_ARP,BTN_ENV,BTN_SEL,BTN_LFO,BTN_GLO,BTN_SAVE,BTN_HOME,BTN_EDIT,BTN_FX,BTN_SEQ,BTN_PLAY,BTN_REC};
static int clampi(int x,int a,int b){return x<a?a:x>b?b:x;}
#define mm_store_load(...) (-1)
static void midi_send(unsigned s,unsigned a,unsigned b){unsigned i=host_midi_count++%256;host_midi_sent[i][0]=s;host_midi_sent[i][1]=a;host_midi_sent[i][2]=b;}
static int midi_send_ready(void){return !host_midi_blocked;}
static void fm1_irq_off(void){}static void fm1_irq_on(void){}
static int smk_extkey_take(void){return -1;}
static int pq[256];static unsigned pw,pr;
static int smk_pad_take(void){return pw==pr?-1:pq[pr++%256];}
static void host_qput(unsigned ignored,int e){(void)ignored;assert(pw-pr<256);pq[pw++%256]=e;}
static void fm1_led_key(unsigned k,unsigned v){(void)k;(void)v;}
static void padled_set(unsigned p,unsigned r,unsigned g,unsigned b){padled_rgb[p]=(r<<16)|(g<<8)|b;}
static void padled_flush(void){}
'''+f'#include "{SRC}/smk_midi_mode.h"\n'+r'''
static void mm_safety_complete(void){}
static void mm_select(unsigned i){if(mm_edit_reserved(i))return;mm_sel=i;mm_pick=0;}
static unsigned mm_pad_colour(unsigned i){unsigned c=mm_profile()->color[i-MM_PAD];if(c)return c;return mm_profile()->map[i].type==MM_NOTE?6:3;}
'''
prefix+='\n'.join(fn(ui,s) for s in ['static void mm_pedal_event(unsigned down){','static void mm_leds(void){','static void mm_pads_poll(void){','static void mm_keys_poll(void){'])
prefix+=r'''
static void mm_input(void){mm_pads_poll();mm_keys_poll();}
static void mm_enter(void){mm_on=1;mm_safety();mm_key_channels=1u<<(mm_main_ch()-1u);}
static void mm_leave(void){mm_safety();mm_on=0;}
'''
case=case[case.index('static unsigned observed,ons;'):]
case=case.replace('engine_init();pf_init();pat_init(&pat);','')
c=prefix+case
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(c)
 subprocess.run(['cc','-std=gnu99','-O2','-Wall','-Wextra','-Wno-unused-function','-Wno-unused-variable','-Wno-misleading-indentation','-I'+str(SRC),str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
