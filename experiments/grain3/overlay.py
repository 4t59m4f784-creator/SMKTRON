"""GR3: remove unreachable FM1 drawing fallback; animate only UI and pad 10."""
from pathlib import Path
import shutil
def apply(R):
 S=R/'firmware/src';H=Path(__file__).parent
 p=S/'mel.c';s=p.read_text()
 marker='''    smktron_settings_leave();
    if (page == PG_ARP || page == PG_SEQ) {
        ui_motion();
        smktron_patterns_draw(base);
        goto smk_drawn;
    }
#endif'''
 assert s.count(marker)==1
 s=s.replace(marker,marker[:-6]+'    goto smk_drawn; /* All SMK37 pages have dedicated renderers. */\n#endif')
 s=s.replace('#include "zoom_ui.h"','#include "grain_visual.h"\n#include "zoom_ui.h"')
 p.write_text(s)
 shutil.copy2(H/'grain_visual.h',S/'grain_visual.h')
 # The controls/focus/persistence functions remain GR2; replace drawing only.
 p=S/'grain_ui.h';s=p.read_text();s=s[:s.index('static void gr_draw(uint32_t base)')]+(H/'grain_draw.h').read_text();p.write_text(s)
 p=S/'mel_smk.h';s=p.read_text();old='''        if (pad != SP_ECHO)
            smk_pad_state(pad, SMK_EFFECT_RGB[pad - SP_LOFI], (p?PV(SMK_FXP[p]):(int32_t)gr_on) && !by_fx, 0, 16u);'''
 new='''        if(!p)gr_pad_visual(tick);
        else if (pad != SP_ECHO)
            smk_pad_state(pad, SMK_EFFECT_RGB[pad - SP_LOFI], PV(SMK_FXP[p]) && !by_fx, 0, 16u);'''
 assert old in s;s=s.replace(old,new);p.write_text(s)
