#!/usr/bin/env python3
"""Exercise actual MIDI note/service and HAL wait code with keys arriving during LCD DMA.
The regression baseline uses the original busy wait without LCD_IDLE. No hardware claims.
"""
from pathlib import Path
import subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'firmware/firmware/src'
HAL=ROOT/'firmware/firmware/hal'
def fn(s,signature):
 a=s.index(signature);b=s.index('{',a);i=b+1;depth=1
 while depth:
  depth+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[a:i]
ui=(SRC/'smk_midi_mode_ui.h').read_text()
wait=fn((HAL/'fm1_lcd_hw.h').read_text(),'static void fm1_lcd_wait(void)')
helpers='\n'.join(fn(ui,s) for s in ['static void mm_keys_poll(void)','static void mm_lcd_idle(void)'])
c=r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
static unsigned mm_on=1,mm_exiting,fm1_ms,irq_depth,sustain_polls,tx_n;
static struct {uint64_t keys;uint8_t key_vel[37],pad_st[16],fader[4];uint32_t buttons;}fm1_in;
static struct {unsigned ms,st,n,v;}tx[512];
enum{BTN_LEFT,BTN_RIGHT,BTN_PLAY,BTN_REC,BTN_SEQ,BTN_FX,BTN_EDIT,BTN_ARP,BTN_LFO,BTN_SEL,BTN_ENV,BTN_GLO,BTN_SAVE};
static int clampi(int x,int lo,int hi){return x<lo?lo:x>hi?hi:x;}
#define mm_store_load(...) (-1)
static int midi_send_ready(void){return 1;}
static void midi_send(unsigned st,unsigned n,unsigned v){assert(tx_n<512);tx[tx_n++]=(typeof(tx[0])){fm1_ms,st,n,v};}
static void fm1_irq_off(void){assert(!irq_depth);irq_depth=1;}
static void fm1_irq_on(void){assert(irq_depth);irq_depth=0;}
static int smk_extkey_take(void){return -1;}
static void mm_sustain_poll(void){sustain_polls++;}
'''+f'#include "{SRC}/smk_midi_mode.h"\n'+helpers+r'''
static unsigned loops,reg,inject,sim_us,release_at;
static unsigned *spi_reg(void){
 sim_us+=100;fm1_ms=sim_us/1000;loops++;
 if(inject&&loops==20){fm1_in.keys=(1ull<<0)|(1ull<<18)|(1ull<<36);fm1_in.key_vel[0]=55;fm1_in.key_vel[18]=93;fm1_in.key_vel[36]=117;}
 if(inject&&loops==release_at)fm1_in.keys=0;
 if(loops>=120)reg|=0x8000;
 return &reg;
}
static unsigned fm1_lcd_timeouts;
#define FM1_LCD_SPI_CON (*spi_reg())
#define LCD_IDLE() mm_lcd_idle()
'''+wait+r'''
static void reset(void){
 memset(&fm1_in,0,sizeof fm1_in);memset(mm_owned,0,sizeof mm_owned);memset(mm_release_type,0,sizeof mm_release_type);
 mm_r=mm_w=mm_panic_left=tx_n=mm_keys_prev=mm_keys_blocked=0;mm_data.selected=0;mm_default(&mm_data.profile[0]);
 mm_on=mm_lcd_live=1;mm_edit=mm_settings=mm_reset_ask=mm_exiting=0;loops=reg=inject=sim_us=0;release_at=10000;fm1_ms+=100;
}
int main(void){
 reset();mm_lcd_live=0;inject=1;fm1_lcd_wait();assert(!tx_n);mm_keys_poll();mm_service();assert(tx_n==3);
 puts("PASS: reproduces MM1f scheduling: a whole chord waits until the LCD finishes");
 reset();inject=1;fm1_lcd_wait();assert(tx_n==3);
 for(unsigned i=0;i<3;i++){assert(tx[i].st==0x90);assert(tx[i].ms<4);}
 assert(tx[0].n==48&&tx[1].n==66&&tx[2].n==84);assert(tx[0].v==55&&tx[1].v==93&&tx[2].v==117);
 unsigned before=tx_n;mm_keys_poll();mm_service();assert(tx_n==before);
 puts("PASS: first, central and last keys transmit during LCD DMA, within 1 ms of the simulated scan, preserving velocity, without duplicates");
 reset();inject=1;release_at=45;fm1_lcd_wait();assert(tx_n==6);
 for(unsigned i=0;i<3;i++){assert(tx[3+i].st==0x80&&tx[3+i].n==tx[i].n);assert(tx[3+i].ms<7);}
 puts("PASS: short taps and releases are preserved even when both occur during one LCD transfer");
 reset();inject=1;mm_edit=1;fm1_lcd_wait();assert(!tx_n);
 mm_edit=0;mm_keys_poll();mm_service();assert(!tx_n);
 puts("PASS: edit mode does not play notes, and leaving edit cannot retrigger a held key");
 reset();fm1_in.keys=mm_keys_blocked=1;fm1_in.key_vel[0]=100;fm1_lcd_wait();assert(!tx_n);
 fm1_in.keys=0;fm1_ms++;mm_lcd_idle();fm1_in.keys=1;fm1_ms++;mm_lcd_idle();assert(tx_n==1);
 puts("PASS: held-key entry blocking and release/repress remain intact");
 reset();inject=1;mm_exiting=1;fm1_lcd_wait();assert(!tx_n);
 reset();inject=1;mm_on=0;fm1_lcd_wait();assert(!tx_n);
 puts("PASS: synth and mode exit cannot emit MIDI through the LCD hook");
 reset();mm_panic_left=80;fm1_lcd_wait();assert(tx_n==80&&!mm_panic_left);
 puts("PASS: complete channel safety reset drains during redraw without queue starvation");
 assert(!fm1_lcd_timeouts&&!irq_depth&&sustain_polls);
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(c)
 subprocess.run(['cc','-std=gnu99','-O2','-Wall','-Wextra','-Wno-unused-function','-Wno-unused-variable','-Wno-misleading-indentation','-I'+str(SRC),str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
