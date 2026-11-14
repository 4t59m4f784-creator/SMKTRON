/* SPDX-License-Identifier: GPL-3.0-only
 * KNOB BANK short release cycles the live pad owners. SAVE/GLOBE remain valid. */

static void pm_buttons(uint32_t b,uint32_t rel){
 if(b&(1u<<BTN_KNOB_BANK)){pm_down=1;pm_time=fm1_ms;}
 if((rel&(1u<<BTN_KNOB_BANK))&&pm_down){
  pm_down=0;
  if(fm1_ms-pm_time>=600u)return;
  if(ch_on){ch_mode(0);page=pm_return;}
  else if(sc_on){sc_pin=0;sc_mode(0);ch_mode(1);}
  else {if(page!=PG_CHORD)pm_return=page;sc_mode(1);sc_pin=1;page=PV(P_BSND)?PG_LAYER:PG_PLAY;}
  redraw=255u;
 }
}
