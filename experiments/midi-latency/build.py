#!/usr/bin/env python3
"""Build MM1h from the frozen MM1f sources plus the LCD/MIDI patch on this branch.
Use --web for WASM; native requires JIELI_TOOLCHAIN and AC79_SDK.
Historical experiments/midi/build.py reconstructs MM1f and is not the MM1h builder.
"""
import os,sys,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
ENGINE=ROOT/'firmware'
flags=['SMKTRON_STRINGS','SMKTRON_PLATE','SMKTRON_USB_STABLE','SMKTRON_MANUAL_SAVE','SMKTRON_FIXED_VELOCITY','SMKTRON_PAD_PALETTE','SMKTRON_REFINEMENTS','SMKTRON_FLUTE_BALANCE','SMKTRON_SCENES']
env=dict(os.environ,BOARD='smk37',MODEL='pro',**{k:'1' for k in flags})
if '--web' in sys.argv:
 (ENGINE/'build/emu').mkdir(parents=True,exist_ok=True)
 cmd=[os.environ.get('ZIG','zig'),'cc','-target','wasm32-freestanding','-O2','-nostdlib','-fno-builtin','-Wall','-Wno-unused-function','-DDINGHY_WEB','-DBOARD_SMK37=1',*[f'-D{x}=1' for x in flags],'-DAPP_VER="0.3.0-mm1h"','-Ifirmware/src','-Ifirmware/gen','-Ibuild/gen','-Wl,--no-entry','-Wl,--export-dynamic','-Wl,-z,stack-size=1048576','-o','build/emu/mel.wasm','firmware/src/mel.c']
else:cmd=['sh','build.sh','--release','0.3.0-mm1h']
subprocess.run(cmd,cwd=ENGINE,env=env,check=True)
