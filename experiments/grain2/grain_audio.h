/* SPDX-License-Identifier: GPL-3.0-only
 * GR1 bounded sample-domain grains. Audio ISR owns heads/RNG; controls are scalar
 * main-loop writes. At most two grains per voice, each with main/layer heads.
 * No capture buffer, allocation, screen or flash work in the audio path. */
static volatile uint32_t gr_on,gr_style,gr_enabled;
static volatile uint32_t gr_ctl[4]={120,35,85,75}; /* SIZE ms, SPREAD %, DENSITY %, MIX % */
typedef struct {head_t a,b;uint32_t age,len,phase,inc;uint8_t layer;} gr_head_t;
static struct {gr_head_t g[2];uint32_t wait;int32_t mix;} gr_voice[NV];
static uint32_t gr_rng=0x7351a9d3u,gr_spawns,gr_frames;
static uint32_t gr_random(void){uint32_t x=gr_rng;x^=x<<13;x^=x>>17;x^=x<<5;return gr_rng=x;}
static void gr_reset(uint32_t v){gr_voice[v].g[0].len=gr_voice[v].g[1].len=0;gr_voice[v].wait=0;gr_voice[v].mix=0;}
static uint32_t gr_position(head_t *dry){
 const mel_zone_t *z=dry->z;int32_t width=(int32_t)gr_ctl[1]*83;
 int32_t p=(int32_t)dry->pos+(((int32_t)(gr_random()&65535u)-32768)*width>>15);
 /* Position spread stays inside the recording; a grain then uses its normal loop. */
 if(p<0)p=0;if((uint32_t)p>=z->n)p=(int32_t)z->n-1;
 return (uint32_t)p;
}
static void gr_seek(head_t *h,head_t *dry){
 /* Align to a decoder checkpoint to avoid a decoding burst at each launch;
  * 256 samples at 11.025 kHz is 23.2 ms of position resolution. */
 head_start(h,dry->z,gr_position(dry),dry->gain);
}

static const char *const GR_STYLE[]={"FREE","BRUMA","PULSOS","FRAGMENTOS"};
