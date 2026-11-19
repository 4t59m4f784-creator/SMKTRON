#!/usr/bin/env python3
"""Build the isolated Elite candidate; never flashes or replaces official Pro artifacts."""
import os, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
flags=['SMKTRON_STRINGS','SMKTRON_PLATE','SMKTRON_USB_STABLE','SMKTRON_MANUAL_SAVE','SMKTRON_FIXED_VELOCITY','SMKTRON_PAD_PALETTE','SMKTRON_REFINEMENTS','SMKTRON_FLUTE_BALANCE','SMKTRON_SCENES']
env=dict(os.environ,BOARD='smk37',MODEL='elite',**{k:'1' for k in flags})
subprocess.run(['sh','build.sh','--release','0.3.0-elite1'],cwd=ROOT/'firmware',env=env,check=True)
