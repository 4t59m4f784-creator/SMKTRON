"""GR2 overlays GR1: persistent effect, K6 styles, modulation-wheel vibrato."""
from pathlib import Path
import shutil
def apply(R):
 S=R/'firmware/src';H=Path(__file__).parent
 p=S/'mel.c';s=p.read_text()
 s=s.replace('#define SET_VER 6u','#define SET_VER 7u')
 s=s.replace('{0, 4, 0, 1}, {0, 5, 3, 1}, {1, 3, 1, 1}, {10, 100, 60, 5}', '{20, 400, 120, 1}, {0, 100, 35, 1}, {15, 100, 85, 1}, {0, 996, 75, 1}')
 s=s.replace('{0, 100, 75, 1}, {0, 100, 35, 1}, {0, 100, 70, 1}, {0, 100, 0, 1}', '{0, 100, 75, 1}, {0, 100, 35, 1}, {0, 100, 70, 1}, {0, 100, 100, 1}')
 marker='static const int16_t EQ_G[25]'
 migration='''/* Same record size: repurpose four retired ARP fields, with a version tag. */
static void gr_migrate(mel_set_t *s){
 if(s->ver==6u||s->ver==5u){s->p[P_AMODE]=120;s->p[P_ARATE]=35;s->p[P_AOCT]=85;s->p[P_AGATE]=75;if(!s->p[P_VMIX])s->p[P_VMIX]=100;s->ver=SET_VER;}
}
'''
 s=s.replace(marker,migration+marker)
 s=s.replace('    int32_t d = PV(P_VDEPTH);', '''    gr_ctl[0]=(uint32_t)PV(P_AMODE);gr_ctl[1]=(uint32_t)PV(P_ARATE);gr_ctl[2]=(uint32_t)PV(P_AOCT);gr_ctl[3]=(uint32_t)PV(P_AGATE)&127u;
    gr_on=((uint32_t)PV(P_AGATE)>>7)&1u;gr_style=((uint32_t)PV(P_AGATE)>>8)&3u;gr_enabled=gr_on&&!by_fx;
    int32_t d = PV(P_VDEPTH);''')
 s=s.replace('s.ver == SET_VER && s.np == NP','(s.ver == SET_VER || s.ver == 6u) && s.np == NP')
 s=s.replace('    if (loaded) {\n        set.bypass', '    if (loaded) {\n        gr_migrate(&s);\n        set.bypass')
 s=s.replace('if ((d = turn(ENC_SELECT)) != 0 && G_COUNT[g] > 1u)', 'if(page==PG_ARP){d=turn(ENC_SELECT);if(d)gr_style_select(d);}\n    else if ((d = turn(ENC_SELECT)) != 0 && G_COUNT[g] > 1u)')
 s=s.replace('fm1_led_key(BTN_ARP, gr_ctl[3]!=0);','fm1_led_key(BTN_ARP, gr_on);')
 s=s.replace('if(ch_on&&page!=PG_ARP)page=PG_CHORD;', 'gr_wheel_focus();\n    if(ch_on&&page!=PG_ARP&&!(gr_focus&&page==PG_VIB))page=PG_CHORD;')
 # Name MAX MIX in every display and header. Wheel position never overwrites it.
 s=s.replace('{P_VMIX, F_PCT, "MIX", 0}', '{P_VMIX, F_PCT, "MAX MIX", 0}')
 s=s.replace('"MODE", ARP_MODE', '"SIZE", 0').replace('{P_AMODE, F_ENUM,','{P_AMODE, F_NUM,').replace('{P_ARATE, F_ENUM, "RATE", ARP_RATE}','{P_ARATE, F_PCT, "SPREAD", 0}').replace('{P_AOCT, F_X, "OCTAVES", 0}','{P_AOCT, F_PCT, "DENSITY", 0}').replace('{P_AGATE, F_PCT, "GATE", 0}','{P_AGATE, F_PCT, "MIX", 0}')
 s+='''\n#if defined(DINGHY_WEB)
EXPORT uint32_t web_grain_state(uint32_t k){return k==0?gr_on:k==1?gr_style:k==2?gr_enabled:0;}
EXPORT void web_grain_style(uint32_t v){if(v<4u)gr_style_select((int32_t)v-(int32_t)gr_style);}
EXPORT uint32_t web_vibrato_mix(void){return (uint32_t)(PV(P_VMIX)*p_mod/127);}
EXPORT void web_grain_on(uint32_t v){gr_on=v!=0;gr_store(99);}
#endif
'''
 p.write_text(s)
 p=S/'mel_ui.c';s=p.read_text();a=s.index('static void stage_arp(void)');b=s.index('static void stage_seq(void)',a);s=s[:a]+'static void stage_arp(void){}\n\n'+s[b:];p.write_text(s)
 p=S/'smktron_scenes.h';s=p.read_text()
 s=s.replace('static int sc_valid(const mel_set_t *s){', 'static int sc_valid(const mel_set_t *saved){\n mel_set_t copy=*saved;gr_migrate(&copy);const mel_set_t *s=&copy;')
 s=s.replace('if(s->ver!=SET_VER||', 'if((((uint32_t)s->p[P_AGATE])&127u)>100u)return 0;\n if(s->ver!=SET_VER||')
 s=s.replace('const mel_set_t *s=&sc_bank.slot[p];uint32_t i;', 'mel_set_t migrated=sc_bank.slot[p];gr_migrate(&migrated);const mel_set_t *s=&migrated;uint32_t i;')
 p.write_text(s)
 p=S/'mel_smk.h';s=p.read_text()
 s=s.replace('} else if (p == SP_LFAST) {', '} else if(p==SP_VIB){gr_toggle();\n        } else if (p == SP_LFAST) {')
 s=s.replace('PV(SMK_FXP[p]) && !by_fx, 0, 16u','(p?PV(SMK_FXP[p]):(int32_t)gr_on) && !by_fx, 0, 16u')
 p.write_text(s)
 p=S/'mel_fx.c';s=p.read_text();a=s.index('    if (p_mod) {');b=s.index('#if SMKTRON_REFINEMENTS',a)
 s=s[:a]+'    vmix=vmix*p_mod/127; /* wheel scales the saved maximum; no depth override */\n'+s[b:];p.write_text(s)
 p=S/'smktron_fx.h';s=p.read_text().replace(',mod=p_mod*100/127','').replace('if(pg==PG_VIB&&mod>mix)mix=mod;', 'if(pg==PG_VIB)mix=mix*p_mod/127;').replace('if(pg==PG_VIB&&!by_fx&&p_mod*100/127>PV(P_VMIX))state="MOD";', 'if(pg==PG_VIB&&!by_fx&&p_mod)state="WHEEL";').replace('p_mod*100/127>PV(P_VMIX)?"MOD WHEEL":mix<70?"CHORUS":"VIBRATO"','"MOD WHEEL"');p.write_text(s)
 p=S/'zoom_ui.h';s=p.read_text().replace(' if(page==PG_APPEAR)', ' if(page==PG_ARP){put(put_int(v,(int32_t)gr_ctl[k],0),k?"%":" ms");}\n else if(page==PG_APPEAR)',1).replace('put(s,page==PG_LAYER?"LAYER OFF":MEL_SET_NAME[st]);','if(page==PG_ARP)put(put(s,"K6 STYLE: "),GR_STYLE[gr_style]);else put(s,page==PG_LAYER?"LAYER OFF":MEL_SET_NAME[st]);').replace('put(put_int(put(s,arp_on?"ARP ON / ":"ARP OFF / "),PV(P_BPM),0)," BPM");','put(s,gr_on?(by_fx?"PAD 10: BYPASS":"PAD 10: ON"):"PAD 10: OFF");').replace('sig=sig*33u+touched;', 'sig=sig*33u+p_mod;sig=sig*33u+touched;').replace(' else if(page==PG_ARP){', ' else if(page==PG_VIB){put(put_int(put(s,"WHEEL MIX "),PV(P_VMIX)*p_mod/127,0),by_fx?"% / BYPASS":"%");}\n else if(page==PG_ARP){');p.write_text(s)
 for n in ['grain_audio.h','grain_render.h','grain_ui.h']:shutil.copy2(H/n,S/n)
