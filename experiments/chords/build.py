#!/usr/bin/env python3
"""CH1 isolated prototype. Generate an overlay; never edit/rebuild AP8 official assets."""
from pathlib import Path
import shutil, subprocess, os, sys
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'experiments/chords'; OUT=ROOT/'dist/chords-preview'
GRAIN3='--grain3' in sys.argv
GRAIN2='--grain2' in sys.argv or GRAIN3
GRAIN='--grain' in sys.argv or '--grain-strip' in sys.argv or GRAIN2
ZOOM='--zoom' in sys.argv or GRAIN
USB_DIAG='--usb-diagnostic' in sys.argv
SCENE='--scenes-audio' in sys.argv or USB_DIAG or ZOOM
AUDIO='--audio' in sys.argv or SCENE
PADS='--pads' in sys.argv or AUDIO
BOOT='--boot' in sys.argv or PADS
VER='0.3.0-gr3' if GRAIN3 else '0.3.0-gr2' if GRAIN2 else '0.3.0-gr1' if GRAIN else '0.3.0-zm1' if ZOOM else '0.3.0-ud1' if USB_DIAG else '0.3.0-at2' if SCENE else '0.3.0-at1' if AUDIO else '0.3.0-pm1' if PADS else '0.3.0-bt2' if BOOT else '0.3.0-ch4'
if BOOT:OUT=ROOT/('dist/audio-preview' if AUDIO else 'dist/pads-preview' if PADS else 'dist/boot-preview')
if USB_DIAG:OUT=ROOT/'dist/usb-diagnostic'
if ZOOM:OUT=ROOT/'dist/zoom-preview'
if GRAIN3:OUT=ROOT/'dist/grain3-preview'
if GRAIN and not GRAIN3:OUT=ROOT/('dist/grain2-preview' if GRAIN2 else 'dist/grain-preview')
R=HERE/'build/engine'; R.parent.mkdir(parents=True,exist_ok=True)
shutil.copytree(ROOT/'experiments/appearance/engine',R,dirs_exist_ok=True,ignore=shutil.ignore_patterns('build','__pycache__'))
S=R/'firmware/src'; p=S/'mel.c'; s=p.read_text()
def replace(a,b):
 global s
 assert a in s,a[:120]
 s=s.replace(a,b)
replace('set.master = 614u;', 'set.master = 921u;') # 90% scene volume; global output remains 70%
replace('volatile uint8_t note, gate;', 'volatile uint8_t note, gate, group;')
replace('static volatile uint32_t tpos[128];', 'static volatile uint32_t tpos[256];\nstatic volatile uint32_t ch_on; static volatile uint8_t ch_sound=1; static volatile uint32_t ch_mix=50; static volatile int32_t ch_pan_m,ch_pan_c;\n#define VPOS(v) tpos[voice[v].note+128u*voice[v].group]\n#define VSET(v) (voice[v].group ? ch_sound : p_sound % MEL_NSETS)\n#define VLAYER(v) (ch_on || voice[v].group ? 0u : p_layer)')
replace('uint32_t st = p_sound % MEL_NSETS, ly = p_layer, note = voice[vi].note', 'uint32_t st = VSET(vi), ly = VLAYER(vi), note = voice[vi].note')
replace('tpos[note]', 'VPOS(vi)')
replace('voice[vi].set != p_sound % MEL_NSETS || voice[vi].layer != p_layer', 'voice[vi].set != VSET(vi) || voice[vi].layer != VLAYER(vi)')
replace('static uint8_t held[128];','static uint8_t held[256];')
replace('i < 128u; i++)                  /* the tapes', 'i < 256u; i++)                  /* the tapes')
replace('held[voice[v].note] = 1;', 'held[voice[v].note+128u*voice[v].group] = 1;')
replace('for (i = 0; i < 128u; i++)\n        if (!held[i]', 'for (i = 0; i < 256u; i++)\n        if (!held[i]')
replace('static void note_on(uint32_t note)', 'static void note_on_group(uint32_t note,uint32_t group)')
replace('voice[v].note == note) {', 'voice[v].note == note && voice[v].group==group) {')
replace('for (v = 0; v < NV; v++) {                  /* else a free voice', 'for (v = ch_on ? (group ? 6u : 0u) : 0u; v < (ch_on && !group ? 6u : NV); v++) {                  /* else a free voice')
replace('voice[pick].note = (uint8_t)note;', 'voice[pick].group=(uint8_t)group;\n    voice[pick].note = (uint8_t)note;')
replace('static void note_off(uint32_t note)', 'static void note_on(uint32_t note){note_on_group(note,0);}\nstatic void note_off(uint32_t note)')
replace('if (voice[v].note == note)\n            voice[v].gate = 0;', 'if (voice[v].note == note && !voice[v].group)\n            voice[v].gate = 0;')
replace('PG_APPEAR, NPG', 'PG_APPEAR, PG_CHORD, NPG')
replace('G_SEQ, G_SETUP};', 'G_SEQ, G_SETUP, G_SETUP};')
replace('{1, 1, 2, 4, 1, 2, 1, 1}', '{1, 1, 2, 4, 1, 3, 1, 1}')
replace('"ARP", "SEQ", "SYSTEM"};', '"ARP", "SEQ", "SYSTEM", "PAD CHORDS"};')
replace('#include "mel_ui.c"', '#include "chords.h"\n#include "mel_ui.c"\n#include "chords_ui.h"')
replace('(page==PG_SETUP?PG_APPEAR:PG_SETUP)', '(page==PG_SETUP?PG_APPEAR:page==PG_APPEAR?PG_CHORD:PG_SETUP)')
replace('if(d%2)page=page==PG_SETUP?PG_APPEAR:PG_SETUP;', 'int32_t ix=page==PG_SETUP?0:page==PG_APPEAR?1:2;ix=((ix+d)%3+3)%3;page=ix==0?PG_SETUP:ix==1?PG_APPEAR:PG_CHORD;')
replace('if(page==PG_APPEAR){\n            d=', 'if(page==PG_CHORD){d=turn(ENC_KNOB1+k);if(d)ch_knob(k,d);continue;}\n        if(page==PG_APPEAR){\n            d=')
replace('    clk_last = fm1_ms;', '    ch_init();\n    clk_last = fm1_ms;')
replace('            rec_note(held_note[k]);', '            rec_note(held_note[k]);') # capture from physical state after all inputs below
replace('    clock_run();', '    ch_capture_poll();\n    clock_run();')
replace('    buttons();\n    knobs();','    if(sc_on && ch_on)ch_mode(0);\n    buttons();\n    knobs();')
replace('    if (page != PG_ARP && page != PG_SEQ)', '    if(page==PG_CHORD){ch_draw(base);goto smk_drawn;}\n    ch_ui_leave();\n    if (page != PG_ARP && page != PG_SEQ)')
replace(' uint32_t k;seq_start(0);', ' ch_stop();ch_hold=0;ch_rec=0;ch_target=255;ch_capture_reset();\n uint32_t k;seq_start(0);')
replace('return pg==PG_APPEAR?', 'return pg==PG_CHORD?(knob==0?(int32_t)ch_mix:knob==1?ch_pan_m:knob==2?ch_pan_c:(int32_t)ch_on):pg==PG_APPEAR?')
replace('static int32_t mix[HALF_FRAMES], act;', 'static int32_t mix[HALF_FRAMES], cmix[HALF_FRAMES], act;')
replace('        mix[i] = 0;', '        mix[i] = cmix[i] = 0;')
replace('voice_sub(v, mix + b,', 'voice_sub(v, (ch_on && voice[v].group ? cmix : mix) + b,')
replace('    fx_run(mix, out, n, act);', '    if(ch_on)ch_bus_mix(mix,cmix,n);\n    fx_run(mix,ch_on?cmix:0,out,n,act);')
replace('#include "mel_fx.c"', '#include "mel_fx.c"\n#include "chords_audio.h"')
s+='\n#include "chords_web.h"\n' 
if BOOT:
 s=s.replace('#include \"chords_ui.h\"','#include \"chords_ui.h\"\n#include \"boot_ui.h\"')
 s=s.replace('    clk_last = fm1_ms;', '    boot_start=fm1_ms;boot_frame=boot_done=0;\n    clk_last = fm1_ms;')
 s=s.replace('    smk_leds();','    boot_draw();if(!boot_leds())smk_leds();')
 s=s.replace('    if(page==PG_CHORD){ch_draw(base);', '    if(!boot_done)goto smk_drawn;\n    if(page==PG_CHORD){ch_draw(base);')
 (S/'boot_ui.h').write_text((HERE/'boot_ui.h').read_text())
 # Model LED transmission, not just pending RGB values, in the boot simulator.
 w=R/'web/emu/fm1_web.c';ws=w.read_text().replace('static uint32_t padled_rgb[16];','static uint32_t padled_rgb[16],padled_committed[16];').replace('static void padled_flush(void){}','static void padled_flush(void){memcpy(padled_committed,padled_rgb,sizeof padled_rgb);}')
 ws=ws.replace('p<16u?padled_rgb[p]:0u','p<16u?padled_committed[p]:0u');w.write_text(ws)
if PADS:
 # Declarations precede mel_smk.h; implementation follows scene functions.
 s=s.replace('#include "mel_smk.h"','static uint32_t pm_down,pm_time,pm_return;\n#include "mel_smk.h"')
 s=s.replace('static void buttons(void)', '#include "pad_modes.h"\nstatic void buttons(void)')
 s=s.replace('    sc_buttons(b,rel);','    pm_buttons(b,rel);\n    sc_buttons(b,rel);')
 s=s.replace('    ch_capture_poll();','    if(!ch_on&&!sc_on&&page!=PG_CHORD)pm_return=page;\n    if(ch_on)page=PG_CHORD;\n    ch_capture_poll();')
 s=s.replace('    ch_ui_leave();','    ch_ui_leave();\n    if(sc_on&&sc_pin){pm_draw(base);goto smk_drawn;}')
 (S/'pad_modes.h').write_text((HERE/'pad_modes.h').read_text().replace('static uint32_t pm_down,pm_time,pm_return;',''))
p.write_text(s)
# Separate two-copy chord store with the proven scene transaction algorithm, new magic and VM range.
t=(S/'scene_storage.h').read_text().replace('sc_','ch_store_').replace('SCENES_AT','CHORDS_AT').replace('0x314E4353u','0x31444843u')
(S/'chord_storage.h').write_text(t)
from stereo_overlay import stereo_fx
(S/'mel_fx.c').write_text(stereo_fx((S/'mel_fx.c').read_text()))
for name in ['chords.h','chords_ui.h','chords_web.h','chords_audio.h']:shutil.copy2(HERE/name,S/name)
p=S/'mel_smk.h';s=p.read_text().replace('        if(sc_pad_event(p,vel))continue;','        if(ch_pad_event(p,vel))continue;\n        if(sc_pad_event(p,vel))continue;').replace('    if(sc_on){sc_leds();goto sc_button_leds;}','    if(ch_on){ch_leds();goto sc_button_leds;}\n    if(sc_on){sc_leds();goto sc_button_leds;}')
from led_overlay import tape_led_helper,scene_leds
if PADS:s=s.replace('if (!(fm1_in.buttons >> BTN_KNOB_BANK & 1u)) {','if (!(fm1_in.buttons >> BTN_KNOB_BANK & 1u) || !pm_down || fm1_ms-pm_time<600u) {')
p.write_text(tape_led_helper(s))
p=S/'smk_store.h';s=p.read_text().replace('#define FL_RANGE_OK(off, n) (', '#define CHORDS_AT 0x000A0000u\n#define CHORDS_END 0x000A2000u\n#define FL_RANGE_OK(off, n) (FL_IN(off,n,CHORDS_AT,CHORDS_END) || ')
s+='\n_Static_assert(CHORDS_AT>=SCENES_END && CHORDS_END<=OWNER_AT,"Chord bank overlaps other data");\n_Static_assert(FL_WIN_SAFE(CHORDS_AT,CHORDS_END),"Chord bank protected range");\n';p.write_text(s)
p=S/'smktron_scenes.h';s=p.read_text().replace('static void sc_mode(uint32_t on){','static void sc_mode(uint32_t on){if(on)ch_mode(0);');
if PADS:s=s.replace('static void sc_mode(uint32_t on){if(on)ch_mode(0);','static void sc_mode(uint32_t on){if(on&&ch_on)ch_mode(0);').replace('page=PV(P_BSND)?PG_LAYER:PG_PLAY;', 'page=PV(P_BSND)?PG_LAYER:PG_PLAY;')
p.write_text(scene_leds(s))
p=S/'smktron_settings.h';s=p.read_text().replace('GLOBAL 2 / 2','GLOBAL 2 / 3').replace('GLOBAL 1 / 2','GLOBAL 1 / 3');p.write_text(s)
if AUDIO:
 import importlib.util
 spec=importlib.util.spec_from_file_location('audio_overlay',ROOT/'experiments/audio-audit/overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R)
 if SCENE:
  spec=importlib.util.spec_from_file_location('scene_overlay',ROOT/'experiments/audio-audit/scene_overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R)
if USB_DIAG:
 spec=importlib.util.spec_from_file_location('usb_diagnostic',ROOT/'experiments/usb-diagnostic/overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R)
if ZOOM:
 spec=importlib.util.spec_from_file_location('zoom_overlay',ROOT/'experiments/zoom/overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R)
if GRAIN:
 spec=importlib.util.spec_from_file_location('grain_overlay',ROOT/'experiments/grain/overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R,strip_only='--grain-strip' in sys.argv)
if GRAIN2:
 spec=importlib.util.spec_from_file_location('grain2_overlay',ROOT/'experiments/grain2/overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R)
if GRAIN3:
 spec=importlib.util.spec_from_file_location('grain3_overlay',ROOT/'experiments/grain3/overlay.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.apply(R)
from compact_system_font import compact
compact(R)
OUT.mkdir(exist_ok=True)
flags=['SMKTRON_STRINGS','SMKTRON_PLATE','SMKTRON_USB_STABLE','SMKTRON_MANUAL_SAVE','SMKTRON_FIXED_VELOCITY','SMKTRON_PAD_PALETTE','SMKTRON_REFINEMENTS','SMKTRON_FLUTE_BALANCE','SMKTRON_SCENES']
if '--native' in sys.argv:
 env=dict(os.environ,BOARD='smk37',MODEL='pro',SMKTRON_USB44='1' if USB_DIAG else '0',**{k:'1' for k in flags})
 log=ROOT/('experiments/grain3' if GRAIN3 else 'experiments/grain2' if GRAIN2 else 'experiments/grain')/('strip-native-build.txt' if '--grain-strip' in sys.argv else 'native-build.txt') if GRAIN else HERE/('zoom-native-build.txt' if ZOOM else 'scene-native-build.txt' if SCENE else 'audio-native-build.txt' if AUDIO else 'pads-native-build.txt' if PADS else 'boot-native-build.txt' if BOOT else 'native-build.txt')
 with log.open('w') as f:r=subprocess.run(['sh','build.sh','--release',VER],cwd=R,env=env,stdout=f,stderr=subprocess.STDOUT)
 print(log.read_text()[-3500:]);sys.exit(r.returncode)
subprocess.run([os.environ.get('ZIG','/workspace/toolchains/zig/ziglang/zig'),'cc','-target','wasm32-freestanding','-O2','-nostdlib','-fno-builtin','-Wall','-Wno-unused-function','-DDINGHY_WEB','-DBOARD_SMK37=1',*[f'-D{x}=1' for x in flags],'-DAPP_VER="'+VER+'"','-Ifirmware/src','-Ifirmware/gen','-Ibuild/gen','-Wl,--no-entry','-Wl,--export-dynamic','-Wl,-z,stack-size=1048576','-o',str(OUT/'mel.wasm'),'firmware/src/mel.c'],cwd=R,check=True)
