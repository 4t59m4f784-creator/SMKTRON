/* SPDX-License-Identifier: GPL-3.0-only.
 * Native SMKTRON USB/TRS fan-out. A host that does not read USB MIDI must
 * never stop the UART. If USB overflows, suppress new USB onsets until an
 * explicit USB-only safety reset can follow the old queued messages.
 * Musical messages still originate only in MIDI MODE.
 * Backend macros allow host tests to run this exact routing code.
 */
#ifndef MP_USB_CONNECTED
#define MP_USB_CONNECTED() (usb.config && !usb.suspended)
#define MP_USB_READY() (mo_w-mo_r<MQ)
#define MP_USB_SEND(st,a,b) midi_out_event(((st)>>4)|((st)<<8)|(((a)&127u)<<16)|(((b)&127u)<<24))
#define MP_USB_IDLE() (mo_w==mo_r)
#define MP_USB_DISCARD() do{fm1_irq_off();mo_r=mo_w;fm1_irq_on();}while(0)
#define MP_TRS_ENTER() do{fm1_irq_off();smk_uart1_tx_init();smk_tx_r=smk_tx_w;smk_tx_busy=0;smk_tx_acked=smk_tx_min_us=smk_tx_pin_high=smk_tx_pin_low=0;fm1_irq_on();}while(0)
#define MP_TRS_READY() smk_tx_ready()
#define MP_TRS_SEND(st,a,b) smk_tx_message(st,a,b)
#define MP_TRS_IDLE() (smk_tx_w==smk_tx_r&&(!smk_tx_busy||smk_uart1_tx_done()))
#endif
static unsigned mp_usb_reset,mp_usb_overruns,mp_leave_at;
static void midi_wire_enter(void){MP_TRS_ENTER();}
static int midi_send_ready(void){return MP_TRS_READY();}
static void mp_usb_poll(void){
 if(!mm_on)return;
 if(!MP_USB_CONNECTED()){mp_usb_reset=0;return;}
 for(unsigned n=0;n<12u&&mp_usb_reset&&MP_USB_READY();n++){
  unsigned i=80u-mp_usb_reset,ch=i/5u,t=i%5u;
  static const uint8_t cc[]={64,123,120,1};
  if(t==3u)MP_USB_SEND(0xE0u|ch,0,64);
  else MP_USB_SEND(0xB0u|ch,cc[t==4u?3u:t],0);
  mp_usb_reset--;
 }
}
static void midi_send(uint32_t st,uint32_t a,uint32_t b){
 if(!mm_on||!MP_TRS_READY())return;
 MP_TRS_SEND(st,a,b);
 if(!MP_USB_CONNECTED()||mp_usb_reset)return;
 if(MP_USB_READY())MP_USB_SEND(st,a,b);
 else{mp_usb_reset=80u;mp_usb_overruns++;}
}
static void midi_wire_begin_leave(void){mp_leave_at=fm1_ms;}
static int midi_wire_idle(void){
 if(!MP_TRS_IDLE())return 0;
 if(!MP_USB_CONNECTED()){MP_USB_DISCARD();mp_usb_reset=0;return 1;}
 if(MP_USB_CONNECTED()&&(!MP_USB_IDLE()||mp_usb_reset)){
  /* Never trap the instrument in controller mode behind an unread USB IN
   * endpoint. The next controller entry starts with a complete safety reset. */
  if(fm1_ms-mp_leave_at<500u)return 0;
  MP_USB_DISCARD();mp_usb_reset=0;
 }
 return 1;
}
