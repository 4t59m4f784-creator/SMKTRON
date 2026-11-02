from pathlib import Path
import shutil

def apply(R):
 S=R/'firmware/src';p=S/'mel.c';s=p.read_text()
 s=s.replace('static unsigned char smktron_theme,','static unsigned char smktron_zoom;\nstatic unsigned char smktron_theme,')
 s=s.replace('smktron_theme=(set.bypass>>5)&1u;', 'smktron_zoom=(set.bypass>>6)&1u;smktron_theme=(set.bypass&96u)!=0;')
 s=s.replace('(set.bypass & 60u)','(set.bypass & 124u)').replace('            63u;','            127u;')
 s=s.replace('value=k?smktron_theme:', 'value=k?(smktron_zoom?2:smktron_theme):').replace('limit=k?1:3','limit=k?2:3').replace('set.bypass&~32u','set.bypass&~96u')
 s=s.replace('knob==1?smktron_theme:','knob==1?(smktron_zoom?2:smktron_theme):')
 s=s.replace('#include "chords_ui.h"','#include "chords_ui.h"\n#include "zoom_ui.h"')
 s=s.replace('    if(page==PG_CHORD){ch_draw(base);','    if(smktron_zoom){zm_draw(base);goto smk_drawn;}\n    if(page==PG_CHORD){ch_draw(base);')
 p.write_text(s)
 p=S/'smktron_scenes.h';s=p.read_text().replace('set.bypass&56u','set.bypass&120u').replace('(smktron_theme<<5)','((smktron_zoom?2u:smktron_theme)<<5)');p.write_text(s)
 p=S/'smktron_settings.h';s=p.read_text().replace('smktron_theme?"DARK":"LIGHT"','smktron_zoom?"ZOOM":smktron_theme?"DARK":"LIGHT"');p.write_text(s)
 shutil.copy2(Path(__file__).parent/'zoom_ui.h',S/'zoom_ui.h')

 from importlib.util import spec_from_file_location,module_from_spec
 spec=spec_from_file_location("zoom_fonts",Path(__file__).parent/"pack_fonts.py");mod=module_from_spec(spec);spec.loader.exec_module(mod);mod.pack(R)
