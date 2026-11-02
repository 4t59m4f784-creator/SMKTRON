/* SPDX-License-Identifier: GPL-3.0-only
 * Render the same panel-check values using the common SMKTRON palette.
 */
static void smktron_diagnostic_draw(const char lines[12][48])
{
    uint32_t tile,i;
    for(tile=0;tile<2u;tile++){
        cv_begin(240,120,SP_BG);cv_oy=-(int32_t)(tile*120u);
        sp_text(12,6,&SP_F10,"SMKTRON",SP_INK,SP_BG);sp_right(228,7,&SP_F8,"PANEL CHECK",SP_MUT,SP_BG);
        cv_rect(12,22,216,1,SP_LINE);
        for(i=1;i<12u;i++){
            int32_t y=29+(int32_t)(i-1u)*15;
            if(!(i&1u))cv_rect(9,y-1,222,14,(smktron_theme?RGB(30,43,35):RGB(214,223,216)));
            sp_text(12,y,&SP_F8,lines[i],i==7u?SP_GREEN:SP_INK,!(i&1u)?(smktron_theme?RGB(30,43,35):RGB(214,223,216)):SP_BG);
        }
        cv_rect(12,198,216,1,SP_LINE);
#if defined(DINGHY_WEB)
        sp_text(12,204,&SP_F7,"SIMULATOR / VIRTUAL READINGS",SP_GREEN,SP_BG);
#else
        sp_text(12,204,&SP_F7,"HARDWARE / LIVE READINGS",SP_GREEN,SP_BG);
#endif
        sp_text(12,217,&SP_F7,"PAD DELTA = RAW - BASE",SP_MUT,SP_BG);
        sp_text(12,230,&SP_F7,"RELEASE KNOB BANK TO RETURN",SP_MUT,SP_BG);
        cv_oy=0;cv_blit(0,tile*120u);
    }
}
