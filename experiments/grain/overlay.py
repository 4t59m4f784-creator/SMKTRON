"""GR1: isolated, sample-domain granular prototype. Never changes released assets."""
from pathlib import Path
import shutil

def strip_arp(S):
 p=S/'mel_seq.c';s=p.read_text()
 a=s.index('static void in_set(');b=s.index('/* ---- the sequencer */')
 s=s[:a]+'''static void in_set(uint32_t note,uint32_t set_b,uint32_t clr_b){if(note<128u)src_set(note,set_b,clr_b);}
static uint8_t in_get(uint32_t note){return note<128u?(uint8_t)(src[note]&S_IN):0;}
static void in_clear(uint32_t bits){uint32_t n;for(n=0;n<128u;n++)if(in_get(n)&bits)in_set(n,0,bits);}
static void arp_release(void){}
static void arp_enable(int on){(void)on;}
'''+s[b:]
 a=s.index('    if (arp_on) {',s.index('static void clock_run'))
 b=s.index('    clk_acc +=',a)
 s=s[:a]+'''    if(!seq_play){clk_acc=0;return;}
'''+s[b:]
 a=s.index('        if (arp_on && arp_n) {',s.index('static void clock_run'))
 b=s.index('        if (seq_play) {',a)
 s=s[:a]+s[b:]
 # Compatibility symbols only: saved records/page numbering keep their old layout.
 s=s.replace('static uint8_t arp_on, seq_play, seq_rec;', 'enum {arp_on=0,arp_cur=255,arp_n=0,arp_pos_shown=0};\nstatic uint8_t seq_play,seq_rec;')
 a=s.index('static uint8_t ain[');b=s.index('static uint32_t seq_step',a)
 s=s[:a]+'static const uint8_t arp_list[1]={0};\n'+s[b:]
 s=s.replace('dt = fm1_ms - clk_last, held = 0, n','dt = fm1_ms - clk_last')
 p.write_text(s)
 p=S/'mel.c';s=p.read_text().replace('ain[k]=0;','').replace(' arp_n=0;arp_i=0;',' ')
 a=s.index('    if (b & (1u << BTN_ARP)) {');b=s.index('    if (b & (1u << BTN_PLAY))',a)
 s=s[:a]+'''    if(b&(1u<<BTN_ARP))go_group(G_ARP);
'''+s[b:]
 s=s.replace('return i<arp_n?arp_list[i]:-1;','(void)i;return -1;')
 p.write_text(s)
 p=S/'smktron_patterns.h';s=p.read_text();a=s.index('static void sq_arp(void)');b=s.index('static void sq_seq(void)',a)
 s=s[:a]+'static void sq_arp(void){}\n'+s[b:]
 s=s.replace('        for(k=0;k<arp_n;k++)sig=(sig^arp_list[k])*16777619u;','')
 p.write_text(s)

def apply(R,strip_only=False):
 S=R/'firmware/src';strip_arp(S)
 if strip_only:return
 here=Path(__file__).parent
 p=S/'mel.c';s=p.read_text()
 s=s.replace('static void voice_start(uint32_t vi)', '#include "grain_audio.h"\nstatic void voice_start(uint32_t vi)')
 s=s.replace('    voice_head(vi);\n    voice[vi].amp = 0;', '    voice_head(vi);\n    gr_reset(vi);\n    voice[vi].amp = 0;')
 s=s.replace('    da = (a1 - a0) / (int32_t)SUB;', '    gr_render(vi,acc,c16);\n    da = (a1 - a0) / (int32_t)SUB;')
 # Renderer needs head_run, which is defined after grain state/reset.
 s=s.replace('static void voice_sub(uint32_t vi,', '#include "grain_render.h"\nstatic void voice_sub(uint32_t vi,')
 s=s.replace('"ARP", "SEQ", "SYSTEM",', '"GRAIN", "SEQ", "SYSTEM",')
 s=s.replace('#include "zoom_ui.h"','#include "zoom_ui.h"\n#include "grain_ui.h"')
 s=s.replace('    if(smktron_zoom){zm_draw(base);', '    if(page==PG_ARP){gr_draw(base);goto smk_drawn;}\n    if(smktron_zoom){zm_draw(base);')
 s=s.replace('        if(page==PG_CHORD){', '        if(page==PG_ARP){d=turn(ENC_KNOB1+k);if(d)gr_knob(k,d);continue;}\n        if(page==PG_CHORD){')
 s=s.replace('fm1_led_key(BTN_ARP, arp_on);','fm1_led_key(BTN_ARP, gr_ctl[3]!=0);')
 s=s.replace('if(ch_on)page=PG_CHORD;', 'if(ch_on&&page!=PG_ARP)page=PG_CHORD;')
 s=s.replace('return pg==PG_CHORD?', 'return pg==PG_ARP?(knob<4u?(int32_t)gr_ctl[knob]:-1):pg==PG_CHORD?')
 s+='''\n#if defined(DINGHY_WEB)
EXPORT void web_grain_set(uint32_t k,int32_t v){if(k<4u)gr_knob(k,v-(int32_t)gr_ctl[k]);}
EXPORT uint32_t web_grain_spawns(void){return gr_spawns;}
EXPORT uint32_t web_grain_frames(void){return gr_frames;}
#endif
'''
 p.write_text(s)
 for name in ['grain_audio.h','grain_render.h','grain_ui.h']:shutil.copy2(here/name,S/name)
