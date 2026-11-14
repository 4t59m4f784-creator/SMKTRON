/* SPDX-License-Identifier: GPL-3.0-only. Simulator and behavioral test probes. */
#if defined(DINGHY_WEB)
EXPORT void web_midi_mode(unsigned on){if(on)mm_enter();else if(mm_on)mm_leave();}
EXPORT int web_mm_state(unsigned k){switch(k){case 0:return mm_on;case 1:return mm_data.selected;case 2:return mm_main_ch();case 3:return mm_relative();case 4:return mm_dirty;case 5:return mm_edit;case 6:return mm_settings;case 7:return mm_kbank;case 8:return mm_fbank;case 9:return mm_pbank;case 10:return mm_exiting;case 11:return mm_panic_left;case 12:return mm_wait_release;default:return -1;}}
EXPORT uint32_t web_mm_tx_count(void){return mm_web_tw-mm_web_tr;}
EXPORT uint32_t web_mm_tx_take(void){if(mm_web_tw==mm_web_tr)return 0;unsigned i=mm_web_tr++%4096;return mm_web_tx[i][0]|mm_web_tx[i][1]<<8|mm_web_tx[i][2]<<16;}
EXPORT uint8_t *web_mm_flash(void){mm_io_init();return mm_flash;}
EXPORT void web_mm_fail(unsigned n){mm_fail_after=n;}
EXPORT void web_mm_save(void){mm_save();}
EXPORT void web_mm_setting(unsigned k,int d){mm_setting_turn(k,d);}
EXPORT void web_mm_control(unsigned i,unsigned type,unsigned num,unsigned ch,unsigned colour){if(i>=MM_N||type>=MM_TYPES||num>127||ch>16||colour>9||mm_reserved(i)||!(mm_types(i)>>type&1u))return;mm_profile()->map[i]=(mm_entry_t){type,num,ch};if(i>=MM_PAD&&i<MM_BTN)mm_profile()->color[i-MM_PAD]=colour;mm_dirty=1;mm_screen_dirty=1;}
EXPORT uint32_t web_mm_entry(unsigned i){if(i>=MM_N)return 0;mm_entry_t e=mm_profile()->map[i];return e.type|e.num<<8|e.ch<<16;}
EXPORT uint32_t web_mm_colour(unsigned i){return i<32?mm_profile()->color[i]:0;}
EXPORT uint32_t web_mm_writes(void){return mm_writes;}
EXPORT void web_mm_panic(void){if(mm_on)mm_safety();else web_panic();}
EXPORT void web_mm_pedal(unsigned raw){fm1_in.pedal=raw;}
EXPORT void web_mm_view(unsigned t){smktron_theme=t==1;smktron_zoom=t==2;mm_screen_dirty=1;redraw=255;}
#endif
