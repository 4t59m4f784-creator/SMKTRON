/* Run the actual native UART ring and USB/TRS router, with independent hosts. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned mm_on=1,fm1_ms,usb_on,uw,ur,wait_ticks,tn;
static uint8_t uq[64][3],wire[16384];
static int smk_uart1_tx_done(void){return wait_ticks==0;}
static void smk_uart1_tx_put(unsigned b){assert(wait_ticks==0);wire[tn++]=b;assert(tn<sizeof wire);wait_ticks=4;}
#define SMK_TX_TICKS() (fm1_ms*2400u)
#define SMK_TX_PIN_HIGH() (wait_ticks&1u)
#define RING_PUBLISH() ((void)0)
#include "vendor/smk_midi_transport.h"
static void usb_send(unsigned st,unsigned a,unsigned b){assert(uw-ur<64);uq[uw%64][0]=st;uq[uw%64][1]=a;uq[uw%64][2]=b;uw++;}
#define MP_USB_CONNECTED() (usb_on)
#define MP_USB_READY() (uw-ur<64)
#define MP_USB_SEND(s,a,b) usb_send(s,a,b)
#define MP_USB_IDLE() (uw==ur)
#define MP_USB_DISCARD() do{ur=uw;}while(0)
#define MP_TRS_ENTER() do{smk_tx_r=smk_tx_w;smk_tx_busy=0;smk_tx_acked=smk_tx_min_us=smk_tx_pin_high=smk_tx_pin_low=0;}while(0)
#define MP_TRS_READY() smk_tx_ready()
#define MP_TRS_SEND(s,a,b) smk_tx_message(s,a,b)
#define MP_TRS_IDLE() (smk_tx_w==smk_tx_r&&!wait_ticks)
#include "midi_ports.h"
static void tick(unsigned usb_read){if(wait_ticks)wait_ticks--;smk_tx_poll();if(usb_read&&ur!=uw)ur++;fm1_ms++;mp_usb_poll();}
static void reset(void){smk_tx_min_us=smk_tx_pin_high=smk_tx_pin_low=smk_tx_started=smk_tx_polls=smk_tx_acked=smk_tx_w=smk_tx_r=smk_tx_busy=tn=uw=ur=wait_ticks=fm1_ms=mp_usb_reset=mp_usb_overruns=mp_leave_at=0;mm_on=1;usb_on=0;}
static void send(unsigned st,unsigned a,unsigned b,unsigned read){while(!midi_send_ready())tick(read);midi_send(st,a,b);tick(read);}
static void drain(unsigned read){for(unsigned i=0;i<8000;i++){tick(read);if(MP_TRS_IDLE()&&(!read||MP_USB_IDLE())&&!mp_usb_reset)return;}}
static void startup(void){for(unsigned i=0;i<80;i++)send(0xB0u|(i/5u),123,0,0);}
int main(void){
 reset();usb_on=1;uw=64;
 assert(!(smk_tx_ready()&&(!usb_on||uw-ur<64))); /* old routing: no MIDI can advance */
 assert(midi_send_ready()); /* fixed routing: TRS progresses even when USB cannot */
 puts("PASS: reproduced old full-USB gate; actual new native router remains ready");
 reset();usb_on=1;startup();send(0x90,60,100,0);send(0x80,60,0,0);drain(0);
 assert(tn==246&&wire[240]==0x90&&wire[241]==60&&wire[242]==100&&wire[243]==0x80);
 assert(mp_usb_overruns==1&&mp_usb_reset==80);
 puts("PASS: 80 entry messages plus note-on/off leave TRS while USB is open but unread");
 drain(1);assert(!mp_usb_reset);send(0x90,62,87,1);assert(uq[(uw-1)%64][0]==0x90&&uq[(uw-1)%64][1]==62);
 puts("PASS: stalled USB recovers through its own 80-message safety reset, without resetting TRS");
 reset();usb_on=1;for(unsigned i=0;i<400;i++)send(i&1?0x80:0x90,36+i%48,i&1?0:90,1);drain(1);assert(!mp_usb_overruns&&tn==1200);
 puts("PASS: active USB and UART sustain 400 complete messages in order");
 reset();for(unsigned i=0;i<80;i++)send(0xB0,1,i,0);drain(0);assert(tn==240&&uw==0);assert(smk_tx_acked==240&&smk_tx_polls>240);assert(smk_tx_min_us==400&&smk_tx_pin_high&&smk_tx_pin_low);
 puts("PASS: TRS works with USB disconnected");
 reset();usb_on=1;startup();drain(0);midi_wire_begin_leave();for(unsigned i=0;i<501;i++)tick(0);assert(midi_wire_idle()&&uw==ur&&!mp_usb_reset);
 mm_on=0;unsigned t=tn,u=uw;send(0x90,64,100,1);for(unsigned i=0;i<100;i++)tick(1);assert(tn==t&&uw==u);
 puts("PASS: unread USB cannot trap mode return; instrument mode emits no musical MIDI");
 reset();usb_on=1;send(0x90,60,100,0);drain(0);assert(uw>ur);usb_on=0;midi_wire_begin_leave();assert(midi_wire_idle()&&uw==ur);
 puts("PASS: suspended/disconnected USB cannot replay stale note-ons after returning to synth");
 reset();send(0x90,60,100,0);drain(0);assert(smk_tx_acked==3);midi_wire_enter();assert(!smk_tx_acked&&!smk_tx_min_us&&!smk_tx_pin_high&&!smk_tx_pin_low);
 puts("PASS: entry resets diagnostic counters; measured completion and pin samples use actual ring");
 return 0;
}
