#!/usr/bin/env python3
"""Compile actual native platform_poll and actual virtual MASTER reader.
WASM emulates encoders and does not run this native competing reader.
"""
from pathlib import Path
import subprocess,tempfile,zipfile
root=Path(__file__).resolve().parents[2]
engine=root/'experiments/midi/build/engine/firmware'
def function(s,signature):
    a=s.index(signature);b=s.index('{',a);depth=1;i=b+1
    while depth:
        depth+=(s[i]=='{')-(s[i]=='}');i+=1
    return s[a:i]
new=function((engine/'src/platform.c').read_text(),'static void platform_poll(void)')
old=(root/'tests/fixtures/k5-mm1e-platform.c').read_text().replace('platform_poll(void)','platform_poll_old(void)')
adc=function((engine/'hal/smk_input.h').read_text(),'static int32_t fm1_adc_read(uint32_t ch)')
c=r'''
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
static unsigned mm_on,fm1_ms,flash_ok=1,services;
static int delta[8],reads[8],smk_master=724,knob_avg=512*16,master_q12=2048;
static struct {int batt;}fm1_in;
static struct {int ota_req,uboot_req;}usb;
static struct {int pending;}bootguard;
enum{FM1_ADC_BATT=3,FM1_ADC_MASTER=4,SMK_ENC_MASTER=4};
static int32_t fm1_enc_take(unsigned e){int d=delta[e];delta[e]=0;reads[e]++;return d;}
#define fm1_wdt_feed() (services++)
#define usb_retry(...) ((void)0)
#define mp_usb_poll() ((void)0)
#define ota_service() ((void)0)
#define ota_session() ((void)0)
#define fm1_audio_stop() ((void)0)
#define lcd_fill(...) ((void)0)
#define draw_text_box(...) ((void)0)
#define usb_detach() ((void)0)
#define fm1_delay_ms(...) ((void)0)
#define fm1_enter_uboot() ((void)0)
#define bootguard_clear(...) ((void)0)
'''+adc+'\n'+old+'\n'+new+r'''
int main(void){
 mm_on=1;delta[4]=7;platform_poll_old();assert(fm1_enc_take(4)==0);
 puts("PASS: MM1e native master reader consumes K5 before MIDI sees its steps");
 smk_master=724;int saved_avg=knob_avg,saved_gain=master_q12;reads[4]=0;
 for(int sign=-1;sign<=1;sign+=2)for(int n=1;n<=8;n++){
  delta[4]=sign*n;delta[0]=3;platform_poll();
  assert(delta[4]==sign*n);assert(fm1_enc_take(4)==sign*n);assert(fm1_enc_take(0)==3);
 }
 assert(smk_master==724&&knob_avg==saved_avg&&master_q12==saved_gain);
 puts("PASS: MIDI retains every K5 step in both directions and preserves synth master/gain");
 for(int n=0;n<100;n++)platform_poll();assert(reads[4]==16);
 puts("PASS: controller idle polling never drains K5; platform services keep running");
 mm_on=0;delta[4]=2;platform_poll();assert(smk_master==772&&delta[4]==0);
 int native_avg=knob_avg,native_gain=master_q12;
 smk_master=724;knob_avg=saved_avg;master_q12=saved_gain;delta[4]=2;platform_poll_old();
 assert(smk_master==772&&knob_avg==native_avg&&master_q12==native_gain);
 assert(services>100);
 puts("PASS: synth K5 volume behavior matches MM1e exactly after leaving MIDI");
}
'''
with tempfile.TemporaryDirectory() as d:
    p=Path(d);(p/'test.c').write_text(c)
    subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Wno-misleading-indentation',str(p/'test.c'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
