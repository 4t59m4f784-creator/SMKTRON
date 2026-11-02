from pathlib import Path

def apply(R):
 S=R/'firmware/src';p=S/'mel.c';s=p.read_text()
 decl='''/* ISR ramps only the dry buses; existing delay/reverb tails keep running. */
static volatile uint32_t scene_gain=32768u,scene_target=32768u,scene_pending=255u;
static void scene_dry_ramp(int32_t *l,int32_t *r,uint32_t n){
 uint32_t i,g=scene_gain,t=scene_target;
 for(i=0;i<n;i++){
  if(g<t)g=g+64u<t?g+64u:t;else if(g>t)g=g>t+64u?g-64u:t;
  l[i]=((l[i]>>15)*(int32_t)g)+(((l[i]&32767)*(int32_t)g)>>15);
  if(r)r[i]=((r[i]>>15)*(int32_t)g)+(((r[i]&32767)*(int32_t)g)>>15);
 }
 scene_gain=g;
}
'''
 s=s.replace('static void app_audio(int32_t *out, uint32_t n)',decl+'\nstatic void app_audio(int32_t *out, uint32_t n)',1)
 s=s.replace('    fx_run(mix,ch_on?cmix:0,out,n,act);','    if(scene_gain!=32768u||scene_target!=32768u)scene_dry_ramp(mix,ch_on?cmix:0,n);\n    fx_run(mix,ch_on?cmix:0,out,n,act);')
 s=s.replace('    if(sc_on&&sc_pin){pm_draw(base);goto smk_drawn;}','')
 s=s.replace('    uint8_t st, d1, d2;','    uint8_t st, d1, d2;\n    scene_service();')
 p.write_text(s)
 p=S/'pad_modes.h';s=p.read_text();s=s[:s.index('static void pm_draw(')];s=s.replace('sc_mode(1);sc_pin=1;','sc_mode(1);sc_pin=1;page=PV(P_BSND)?PG_LAYER:PG_PLAY;');p.write_text(s)
 p=S/'smktron_scenes.h';s=p.read_text()
 s=s.replace('static void sc_mode(uint32_t on){','static void sc_mode(uint32_t on){if(!on){scene_pending=255u;scene_target=32768u;}')
 s=s.replace('static void sc_recall(uint32_t p){','static void sc_apply(uint32_t p){',1)
 marker='static void sc_save(uint32_t p){'
 handler='''static void sc_recall(uint32_t p){
 if(!(sc_bank.mask&(1u<<p))){sc_notice("EMPTY ",p);return;}
 if(!sc_valid(&sc_bank.slot[p])){sc_notice("INVALID ",p);return;}
 scene_pending=p;scene_target=0u;
}
static void scene_service(void){
 if(scene_pending<8u&&scene_gain==0u){
  uint32_t p=scene_pending;scene_pending=255u;sc_apply(p);scene_target=32768u;
 }
}
'''
 assert marker in s;s=s.replace(marker,handler+marker);p.write_text(s)
 p=S/'chords_web.h';p.write_text(p.read_text()+'''\n#if defined(DINGHY_WEB)
EXPORT uint32_t web_scene_gain(void){return scene_gain;}
EXPORT uint32_t web_scene_pending(void){return scene_pending;}
#endif
''')
