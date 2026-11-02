from pathlib import Path

def apply(R):
 S=R/'firmware/src'
 p=S/'mel_fx.c';s=p.read_text()
 s=s.replace('static fx_mix_ramp_t vm, lm, em;', 'static fx_mix_ramp_t vm, lm, em, rm;')
 s=s.replace('    ovp_plate_block(n, rmix);','    fx_mix_target(&rm,plate_mix*256);\n    ovp_plate_block(n, rmix);')
 s=s.replace('        vmix = fx_mix_tick(&vm);','        plate_mix=fx_mix_tick(&rm);\n        vmix = fx_mix_tick(&vm);')
 for c in ['l','r']:
  v='ovp_rv_y'+c
  s=s.replace(f'{c}+=({v}*plate_mix)>>8;',f'{c}+=(({v}*(plate_mix>>8))>>8)+(({v}*(plate_mix&255))>>16);')
 s=s.replace('        if (vmix) {','        { /* Keep delay history warm at zero wet level. */')
 s=s.replace('    if (!vmix) fxv_vib += vinc * n;', '    if (0) fxv_vib += vinc * n;')
 # Warm the delay even during the global FX bypass; mix only afterward.
 a=s.index('        { /* Keep delay history warm at zero wet level. */')
 b=s.index('            y += ((vt - y) * vmix)',a)
 history=s[a:b]+'        }\n'
 s=s[:a]+'        {\n'+s[b:]
 marker='        if (bf >= 32640 && bft)'
 s=s.replace(marker,history+marker)
 p.write_text(s)
 # Exactly the same stereo-linked smoothing in hardware ISR and web adapter.
 for p in [S/'platform.c',R/'web/emu/fm1_web.c']:
  s=p.read_text();anchor='static volatile int32_t master_q12 = 2048;' if p.name=='platform.c' else 'static int32_t master_q12 = 2048;'
  helper='''
static int32_t audio_gain_tick(int32_t target){
 static int32_t value;static uint32_t init;int32_t d;
 target*=256;if(!init){value=target;init=1;}
 d=target-value;if(d>-256&&d<256)value=target;else value+=d/256;
 return value/256;
}
'''
  assert anchor in s;s=s.replace(anchor,anchor+helper+(helper.replace('audio_gain_tick','audio_scene_tick') if p.name!='platform.c' else ''))
  if p.name=='platform.c':
   s=s.replace('            int32_t v = (o[i] * output_gain) >> 12;', '            if(!(i&1u))smooth_gain=audio_gain_tick(output_gain);\n            int32_t v = (o[i] * smooth_gain) >> 12;').replace('        int32_t output_gain=','        int32_t smooth_gain=0;\n        int32_t output_gain=')
  else:
   s=s.replace('        int32_t output_gain=','        int32_t smooth_gain=0;\n        int32_t output_gain=').replace('            int32_t v = (blk[i] * output_gain) >> 12;', '            if(!(i&1u))smooth_gain=audio_gain_tick(output_gain);\n            int32_t v = (blk[i] * smooth_gain) >> 12;')
  if p.name!='platform.c':s=s.replace('v=(v*smk_master)>>10;', 'if(!(i&1u))scene_gain=audio_scene_tick(smk_master);v=(v*scene_gain)>>10;').replace('int32_t smooth_gain=0;', 'int32_t smooth_gain=0,scene_gain=0;')
  p.write_text(s)
