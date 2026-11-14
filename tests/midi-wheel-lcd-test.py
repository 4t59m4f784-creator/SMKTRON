#!/usr/bin/env python3
"""Native regression: actual 14-bit wheel polling, MIDI service and SPI wait.
Simulated ADC ramps must produce multiple bend updates before a redraw ends.
"""
from pathlib import Path
import tempfile,subprocess
ROOT=Path(__file__).resolve().parents[1]
ns={'__file__':str(ROOT/'tests/midi-lcd-latency-test.py')}
exec((ROOT/'tests/midi-lcd-latency-test.py').read_text().split('with tempfile.TemporaryDirectory() as d:')[0],ns)
c=ns['c'];c=c[:c.index('int main(void)')]
c=c.replace('uint32_t buttons;}fm1_in;','uint32_t buttons,frames;uint16_t wheel[2];}fm1_in;')
c=c.replace('static void mm_wheels_poll(void){}','static void mm_wheels_poll(void);')
actual=ns['fn']((ns['SRC']/'smk_midi_mode_ui.h').read_text(),'static void mm_wheels_poll(void){')
idx=c.index('/* Shared by') if '/* Shared by' in c else c.index('static void mm_keys_poll(void)')
c=c[:idx]+'static smk_pw_t smk_pitch_calibration;\nstatic void mm_select(unsigned i){mm_sel=i;mm_pick=0;mm_screen_dirty=1;}\n'+actual+'\n'+c[idx:]
c=c.replace('if(inject&&loops==20){','if(inject&&loops%10==0)fm1_in.wheel[0]=(uint16_t)(500+loops);\n if(inject&&loops==20){')
c+=r'''
static unsigned bends(void){unsigned n=0;for(unsigned i=0;i<tx_n;i++)if((tx[i].st&0xf0)==0xe0)n++;return n;}
static unsigned bend(unsigned i){return tx[i].n|(tx[i].v<<7);}
static void init(void){reset();fm1_in.frames=400;fm1_in.wheel[0]=500;fm1_in.wheel[1]=615;smk_pitch_calibration=(smk_pw_t){0};mm_wheel_prev[0]=mm_wheel_prev[1]=-1;mm_profile()->ch=7;mm_keys_blocked=~0ull;mm_wheels_poll();mm_service();tx_n=0;}
int main(void){
 init();mm_lcd_live=0;inject=1;fm1_lcd_wait();assert(bends()==0);mm_wheels_poll();mm_service();assert(bends()==1);
 puts("PASS: reproduces MM1g wheel starvation during LCD DMA: no intermediate bends");
 init();inject=1;fm1_lcd_wait();unsigned n=bends(),last=8192,lowbits=0;assert(n>=5);
 for(unsigned i=0;i<tx_n;i++)if((tx[i].st&0xf0)==0xe0){assert(tx[i].st==0xe6);assert(tx[i].ms<12);unsigned v=bend(i);assert(v>=last&&v<=16383);last=v;lowbits|=v&127u;}
 assert(lowbits);unsigned before=tx_n;mm_wheels_poll();mm_service();assert(tx_n==before);
 fm1_in.wheel[0]=500;fm1_ms++;mm_lcd_idle();assert(bend(tx_n-1)==8192);
 puts("PASS: multiple monotonic, full-14-bit bends on selected channel during DMA, no duplicates and exact centre return");
 init();mm_profile()->map[MM_PITCH].type=MM_CC;mm_profile()->map[MM_PITCH].num=74;inject=1;fm1_lcd_wait();unsigned cc=0;
 for(unsigned i=0;i<tx_n;i++)if(tx[i].st==0xb6&&tx[i].n==74)cc++;
 assert(cc&&bends()==0);puts("PASS: intentionally remapped pitch wheel still sends CC instead of bend");
 init();mm_edit=mm_pick=1;mm_sel=5;inject=1;fm1_lcd_wait();assert(tx_n==0&&mm_sel==5&&mm_pick==1);
 puts("PASS: redraw callback cannot select controls or send wheels in editor");
 init();mm_profile()->map[MM_PITCH].type=MM_OFF;inject=1;fm1_lcd_wait();assert(bends()==0);
 puts("PASS: disabled pitch mapping stays disabled during DMA");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(c)
 subprocess.run(['cc','-std=gnu99','-O2','-Wall','-Wextra','-Wno-unused-function','-Wno-unused-variable','-Wno-misleading-indentation','-I'+str(ns['SRC']),str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
