#!/usr/bin/env python3
"""Build isolated SMKTRON MM1 from the generated, approved GR3 source."""
from pathlib import Path
import shutil, subprocess, os, sys
ROOT=Path(__file__).resolve().parents[2]
HERE=Path(__file__).parent
ENGINE=HERE/'build/engine'
BASE=ROOT/'experiments/chords/build/engine'
SOURCE=HERE/'vendor'
if not BASE.exists():
    subprocess.run([sys.executable,str(ROOT/'experiments/chords/build.py'),'--grain3'],check=True)
shutil.copytree(BASE,ENGINE,dirs_exist_ok=True,ignore=shutil.ignore_patterns('build','__pycache__'))
S=ENGINE/'firmware/src'
def edit(p,a,b):
    s=p.read_text(); assert a in s, (str(p),a[:100]); p.write_text(s.replace(a,b))
core=(SOURCE/'smk_midi_mode.h').read_text()
core=core.replace('#define mm_on smk_controller_active','')
core=core.replace('static smk_pw_t smk_pitch_calibration;', '/* pitch calibration shared with the instrument */')
core=core.replace('mm_values[16],','mm_values[16],')
(S/'smk_midi_mode.h').write_text(core)
shutil.copy2(SOURCE/'smk_uart_tx.h',ENGINE/'firmware/hal/smk_uart_tx.h')
shutil.copy2(SOURCE/'smk_midi_transport.h',S/'smk_midi_transport.h')
for name in ['midi_storage.h','midi_bridge.h','midi_draw.h','midi_ports.h']:
    shutil.copy2(HERE/name,S/name)
ui=(SOURCE/'smk_midi_mode_ui.h').read_text()
a=ui.index('static void mm_discard_midi');b=ui.index('static void mm_setting_turn',a)
ui=ui[:a]+ui[b:]
a=ui.index('static void mm_draw(void)');b=ui.index('static void mm_input(void)',a)
ui=ui[:a]+'#include "midi_draw.h"\n'+ui[b:]
ui=ui.replace('smk_pedal','mm_pedal')
ui=ui.replace('smk_sustain_poll();','mm_sustain_poll();')
ui=ui.replace('if(!mm_on)return;','if(!mm_on||mm_exiting)return;')
ui=ui.replace('smk_pw_bend(&smk_pitch_calibration,fm1_in.wheel[0],fm1_ms)+8192','smk_pw_bend(&smk_pitch_calibration,fm1_in.wheel[0],fm1_ms)+8192')
ui=ui.replace('static smk_pw_t smk_pitch_calibration;', 'static smk_pw_t smk_pitch_calibration;')
ui=ui.replace(' padled_flush();',' padled_flush();')
# Global pad brightness belongs to the instrument in both modes.
ui=ui.replace('MM_COLOR[c][0]*level/255u','MM_COLOR[c][0]*level*smktron_pad_scale/1020u').replace('MM_COLOR[c][1]*level/255u','MM_COLOR[c][1]*level*smktron_pad_scale/1020u').replace('MM_COLOR[c][2]*level/255u','MM_COLOR[c][2]*level*smktron_pad_scale/1020u')
ui=ui.replace('static void mm_poll(void){', 'static void mm_poll(void){\n if(mm_exiting){mm_service();mm_discard_midi();if(!mm_panic_left&&mm_w==mm_r&&midi_wire_idle())mm_finish_leave();return;}')
(S/'smk_midi_mode_ui.h').write_text(ui)
p=S/'mel.c'
edit(p,'#include "platform.c"','static volatile unsigned int mm_on,mm_fx_reset;\nstatic unsigned int mm_wheel_sync;\nstatic void mm_boot(void);\n#include "platform.c"')
edit(p,'static void app_audio(int32_t *out, uint32_t n)\n{','static void app_audio(int32_t *out, uint32_t n)\n{\n    if(mm_on){memset(out,0,n*2u*sizeof *out);return;}')
edit(p,'PG_APPEAR, PG_CHORD, NPG','PG_APPEAR, PG_CHORD, PG_MIDI, NPG')
edit(p,'G_SEQ, G_SETUP, G_SETUP};','G_SEQ, G_SETUP, G_SETUP, G_SETUP};')
edit(p,'{1, 1, 2, 4, 1, 3, 1, 1}','{1, 1, 2, 4, 1, 4, 1, 1}')
edit(p,'"SYSTEM", "PAD CHORDS"};','"SYSTEM", "PAD CHORDS", "MIDI MODE"};')
edit(p,'page==PG_APPEAR?PG_CHORD:PG_SETUP','page==PG_APPEAR?PG_CHORD:page==PG_CHORD?PG_MIDI:PG_SETUP')
edit(p,'int32_t ix=page==PG_SETUP?0:page==PG_APPEAR?1:2;ix=((ix+d)%3+3)%3;page=ix==0?PG_SETUP:ix==1?PG_APPEAR:PG_CHORD;',
     'int32_t ix=page==PG_SETUP?0:page==PG_APPEAR?1:page==PG_CHORD?2:3;ix=((ix+d)%4+4)%4;page=ix==0?PG_SETUP:ix==1?PG_APPEAR:ix==2?PG_CHORD:PG_MIDI;')
edit(p,'static void buttons(void)','#include "midi_bridge.h"\nstatic void buttons(void)')
edit(p,'    ch_init();','    ch_init();\n    mm_boot();')
edit(p,'        if(page==PG_ARP){d=turn(ENC_KNOB1+k);','        if(page==PG_MIDI){d=fm1_enc_take(ENC_KNOB1+k);if(d&&k==0)mm_setting_turn(k,d);continue;}\n        if(page==PG_ARP){d=turn(ENC_KNOB1+k);')
edit(p,'    scene_service();','    if(mm_on){mm_poll();return;}\n    mm_sustain_poll();\n    if(mm_wait_release){mm_discard_midi();mm_drain_panel();if(fm1_in.keys||mm_any_pad()||mm_pedal.candidate)return;mm_wait_release=0;prev_keys=0;}\n    scene_service();')
edit(p,'    knobs();','    knobs();\n    if(mm_on){mm_poll();return;}')
edit(p,'if(ch_on&&page!=PG_ARP&&','if(ch_on&&page!=PG_MIDI&&page!=PG_ARP&&')
# Musical USB input is intentionally ignored in instrument mode. OTA uses its own SysEx path.
edit(p,'    while (midi_get(&st, &d1, &d2)) {','    mm_discard_midi();\n    while (midi_get(&st, &d1, &d2)) {')
edit(p,'    if(!boot_done)goto smk_drawn;','    if(page==PG_MIDI){mm_entry_draw();goto smk_drawn;}\n    if(!boot_done)goto smk_drawn;')
edit(p,'EXPORT uint32_t web_page_count(void){return NPG;}','EXPORT uint32_t web_page_count(void){return NPG;}')
p.write_text(p.read_text()+'\n#include "midi_web.h"\n')
shutil.copy2(HERE/'midi_web.h',S/'midi_web.h')
p=S/'mel_smk.h'
edit(p,'#include "smk_wheel.h"','#include "smk_wheel.h"\nstatic smk_pw_t smk_pitch_calibration;')
edit(p,'    static smk_pw_t pw;','')
edit(p,'smk_pw_bend(&pw,','smk_pw_bend(&smk_pitch_calibration,')
edit(p,'if (b != last_b)', 'if (mm_wheel_sync || b != last_b)')
edit(p,'if (mw != last_m) { last_m = mw; p_mod = mw; }','if (mm_wheel_sync || mw != last_m) { last_m = mw; p_mod = mw; }\n    mm_wheel_sync=0;')
p=S/'mel_fx.c'
edit(p,'    int32_t age = f_age,','    if(mm_fx_reset){memset(tape,0,sizeof tape);hiss=hiss2=am=vt=les_x=e_lp=0;drop_g=32768;vw=li=ew=0;mm_fx_reset=0;}\n    int32_t age = f_age,')
p=ENGINE/'tools/build.py'
edit(p,'    fm1pkg_make.SDK = a.sdk','    PRODUCT="SMK-37 Pro_460" # MM1f, GR3 stays 453, MM1 stays 455\n    fm1pkg_make.SDK = a.sdk')
p=S/'smk_store.h'
edit(p,'#define FL_RANGE_OK(off, n) (','#define MIDI_MAP_AT 0x000A2000u\n#define MIDI_MAP_END 0x000A4000u\n#define FL_RANGE_OK(off, n) (FL_IN(off,n,MIDI_MAP_AT,MIDI_MAP_END) || ')
p.write_text(p.read_text()+'\n_Static_assert(MIDI_MAP_AT>=CHORDS_END && MIDI_MAP_END<=OWNER_AT,"MIDI maps overlap data");\n_Static_assert(FL_WIN_SAFE(MIDI_MAP_AT,MIDI_MAP_END),"MIDI maps protected range");\n')
p=S/'platform.c'
edit(p,'#include "usb.c"','#include "usb.c"\n#define RING_PUBLISH() __asm__ volatile("" ::: "memory")\n#include "smk_uart_tx.h"\n#include "smk_midi_transport.h"')
s=p.read_text();a=s.index('static void midi_send(uint32_t st,');b=s.index('/* ---- the timer:',a);s=s[:a]+'#include "midi_ports.h"\n\n'+s[b:];p.write_text(s)
edit(p,'    fm1_input_tick();','    fm1_input_tick();\n    smk_tx_poll();')
edit(p,'    usb_start();\n    fm1_timer5_start','    usb_start();\n    smk_uart1_tx_init();\n    fm1_timer5_start')
edit(p,'    usb_retry(fm1_ms);','    usb_retry(fm1_ms);\n    mp_usb_poll();')
# The synth's virtual MASTER pot consumes physical K5 events. MIDI MODE owns
# all encoders; the synth must not drain K5 before mm_input sees it.
edit(p,'if ((a = fm1_adc_read(FM1_ADC_MASTER)) >= 0)',
       'if (!mm_on && (a = fm1_adc_read(FM1_ADC_MASTER)) >= 0)')
# Emulator observes actual MIDI output and stores maps independently.
p=ENGINE/'web/emu/fm1_web.c'
edit(p,'uint8_t key_vel[37], fader[4];','uint8_t key_vel[37], fader[4],pad_st[16];')
edit(p,'uint16_t wheel[2], batt,','uint16_t wheel[2], pedal, batt,')
edit(p,'static void midi_send(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }',
     '''static uint8_t mm_web_tx[4096][3];static uint32_t mm_web_tw,mm_web_tr;
static int midi_send_ready(void){return mm_web_tw-mm_web_tr<4096u;}
static int midi_wire_idle(void){return 1;}
static void midi_wire_begin_leave(void){}
static void midi_wire_enter(void){}
static void midi_send(uint32_t st,uint32_t a,uint32_t b){if(!mm_on||!midi_send_ready())return;unsigned i=mm_web_tw++%4096u;mm_web_tx[i][0]=st;mm_web_tx[i][1]=a;mm_web_tx[i][2]=b;}
static void fm1_irq_off(void){}static void fm1_irq_on(void){}''')
# actual pad state is needed when blocking held controls across transitions
s=p.read_text();a=s.index('EXPORT void web_pad(');b=s.index('{',a)+1;s=s[:b]+'\n if(p<16u)fm1_in.pad_st[p]=vel?0x80:0;'+s[b:];p.write_text(s)
OUT=ROOT/'dist/midi-preview';OUT.mkdir(exist_ok=True)
flags=['SMKTRON_STRINGS','SMKTRON_PLATE','SMKTRON_USB_STABLE','SMKTRON_MANUAL_SAVE','SMKTRON_FIXED_VELOCITY','SMKTRON_PAD_PALETTE','SMKTRON_REFINEMENTS','SMKTRON_FLUTE_BALANCE','SMKTRON_SCENES']
if '--native' in sys.argv:
    env=dict(os.environ,BOARD='smk37',MODEL='pro',**{k:'1' for k in flags})
    with (HERE/'native-build.txt').open('w') as f:
        r=subprocess.run(['sh','build.sh','--release','0.3.0-mm1f'],cwd=ENGINE,env=env,stdout=f,stderr=subprocess.STDOUT)
    print((HERE/'native-build.txt').read_text()[-7000:]);sys.exit(r.returncode)
subprocess.run([os.environ.get('ZIG','/workspace/scratch/mm11-deps/python/ziglang/zig'),'cc','-target','wasm32-freestanding','-O2','-nostdlib','-fno-builtin','-Wall','-Wno-unused-function','-DDINGHY_WEB','-DBOARD_SMK37=1',*[f'-D{x}=1' for x in flags],'-DAPP_VER="0.3.0-mm1f"','-Ifirmware/src','-Ifirmware/gen','-Ibuild/gen','-Wl,--no-entry','-Wl,--export-dynamic','-Wl,-z,stack-size=1048576','-o',str(OUT/'mel.wasm'),'firmware/src/mel.c'],cwd=ENGINE,check=True)
