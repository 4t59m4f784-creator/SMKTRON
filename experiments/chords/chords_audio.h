/* SPDX-License-Identifier: GPL-3.0-only
 * Equal-power crossfade; balance pans (center = unity, no extra gain).
 * Per-sample 20 ms ramps. Mono input retained exactly when chord mode is off.
 * Separate stereo tape/TONE/EQ states, ONE shared modulation/delay/reverb tank.
 */
static int32_t ch_gain(int32_t x,int32_t g){return (x>>15)*g+(((x&32767)*g)>>15);}
static void ch_bus_mix(int32_t *melody,int32_t *chords,uint32_t n){
 static fx_mix_ramp_t gain[4];
 int32_t m=sine(0x40000000u-ch_mix*(0x40000000u/100u));
 int32_t c=sine(ch_mix*(0x40000000u/100u));
 uint32_t i;int32_t pm=ch_pan_m,pc=ch_pan_c;
 if(ch_mix==100u)m=0;if(!ch_mix)c=0;
 fx_mix_target(&gain[0],m*(100-(pm>0?pm:0))/100);
 fx_mix_target(&gain[1],m*(100-(pm<0?-pm:0))/100);
 fx_mix_target(&gain[2],c*(100-(pc>0?pc:0))/100);
 fx_mix_target(&gain[3],c*(100-(pc<0?-pc:0))/100);
 for(i=0;i<n;i++){
  int32_t a=melody[i],b=chords[i];
  melody[i]=ch_gain(a,fx_mix_tick(&gain[0]))+ch_gain(b,fx_mix_tick(&gain[2]));
  chords[i]=ch_gain(a,fx_mix_tick(&gain[1]))+ch_gain(b,fx_mix_tick(&gain[3]));
 }
}
