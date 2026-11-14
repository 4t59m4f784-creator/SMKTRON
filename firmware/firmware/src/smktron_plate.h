/* SPDX-License-Identifier: GPL-3.0-only
 * Plate DSP derived from zvenson/OberVave obv_fx.h, Copyright (C) 2026 zvenson.
 * Upstream commit 7e1ccf9ca9e96a14a961e48ea8fe6efa907422df.
 * Jon Dattorro's 1997 figure-of-eight plate. SMKTRON integration: reverb only,
 * full 44100 Hz, upstream int16 lines, coefficient/tap/modulation arithmetic.
 * Source and attribution: experiments/violins-plate/UPSTREAM.md.
 */
#define REV_DIV 1u
#define RVL(x) ((uint32_t)((x) * 44100u / 29761u / REV_DIV))   /* the paper's 29761 Hz lengths at our rate */
#define REV_PRE_N (8192u / REV_DIV)             /* up to 120 ms of pre-delay */
#define RV_IN1 RVL(142)
#define RV_IN2 RVL(107)
#define RV_IN3 RVL(379)
#define RV_IN4 RVL(277)
#define RV_AP1 RVL(672)
#define RV_D1 RVL(4453)
#define RV_AP2 RVL(1800)
#define RV_D2 RVL(3720)
#define RV_AP3 RVL(908)
#define RV_D3 RVL(4217)
#define RV_AP4 RVL(2656)
#define RV_D4 RVL(3163)
#define REV_N (REV_PRE_N + RV_IN1 + RV_IN2 + RV_IN3 + RV_IN4 + RV_AP1 + 16u + RV_D1 + RV_AP2 + RV_D2 + RV_AP3 + 16u + RV_D3 + RV_AP4 + RV_D4)
#define RV_BW (REV_DIV == 2u ? 24000 : 16700)   /* the input's one-pole: ~5 kHz either way */
typedef struct { int16_t *b; uint32_t len, w; } ovp_line_t;   /* w: where the next sample goes */
static int16_t ovp_fx_mem[REV_N] __attribute__((section(".pool")));
static uint32_t ovp_fx_used;

static inline int32_t ovp_sat16(int32_t v) { return v > 32767 ? 32767 : v < -32767 ? -32767 : v; }
static void ovp_line_init(ovp_line_t *l, uint32_t len) { l->b = ovp_fx_mem + ovp_fx_used; l->len = len; l->w = 0; ovp_fx_used += len; }
static inline __attribute__((always_inline)) int32_t ovp_line_read(const ovp_line_t *l, uint32_t back)   /* 1 <= back <= len */
{
    uint32_t i = l->w + l->len - back;
    if (i >= l->len) i -= l->len;
    return l->b[i];
}
static inline __attribute__((always_inline)) void ovp_line_write(ovp_line_t *l, int32_t v)
{
    l->b[l->w] = (int16_t)ovp_sat16(v);
    if (++l->w >= l->len) l->w = 0;
}
/* Dattorro's allpass: the line holds the inner node, the output is tapped from it. g Q15, signed */
static inline __attribute__((always_inline)) int32_t ovp_ap_step(ovp_line_t *l, int32_t in, int32_t g)
{
    int32_t d = ovp_line_read(l, l->len), v = in - ((d * g) >> 15);
    ovp_line_write(l, v);
    return d + (int32_t)(((int64_t)v * g) >> 15);
}
/* the same with the delay swaying by e (Q8 samples, |e| <= 8 << 8) around base; the line is base + 16 long */
static inline __attribute__((always_inline)) int32_t ovp_ap_mod(ovp_line_t *l, int32_t in, int32_t g, uint32_t base, int32_t e)
{
    uint32_t back = (uint32_t)((int32_t)base + (e >> 8));
    int32_t f = e & 255, a = ovp_line_read(l, back), b = ovp_line_read(l, back + 1u), d = a + (((b - a) * f) >> 8), v;
    v = in - ((d * g) >> 15);
    ovp_line_write(l, v);
    return d + (int32_t)(((int64_t)v * g) >> 15);
}
static inline int32_t ovp_tri_q(uint32_t ph)        /* a triangle -2048..2047 (so Q8 = +-8 samples) */
{
    int32_t t = (int32_t)(ph >> 19);            /* 0..8191 */
    return t < 4096 ? t - 2048 : 6143 - t;
}

static ovp_line_t ovp_rv_pre, ovp_rv_in[4], ovp_rv_ap1, ovp_rv_d1, ovp_rv_ap2, ovp_rv_d2, ovp_rv_ap3, ovp_rv_d3, ovp_rv_ap4, ovp_rv_d4;
static uint32_t ovp_rv_ph1, ovp_rv_ph2;
static volatile int32_t ovp_rev_decay, ovp_rev_damp_k;
static int32_t ovp_rv_lp_in, ovp_rv_lpl, ovp_rv_lpr, ovp_rv_yl, ovp_rv_yr;
static volatile uint32_t ovp_rev_pre_n = 1;
static uint32_t ovp_rev_active;
static void ovp_plate_init(void) {
    ovp_line_init(&ovp_rv_pre, REV_PRE_N);
    ovp_line_init(&ovp_rv_in[0], RV_IN1); ovp_line_init(&ovp_rv_in[1], RV_IN2); ovp_line_init(&ovp_rv_in[2], RV_IN3); ovp_line_init(&ovp_rv_in[3], RV_IN4);
    ovp_line_init(&ovp_rv_ap1, RV_AP1 + 16u); ovp_line_init(&ovp_rv_d1, RV_D1); ovp_line_init(&ovp_rv_ap2, RV_AP2); ovp_line_init(&ovp_rv_d2, RV_D2);
    ovp_line_init(&ovp_rv_ap3, RV_AP3 + 16u); ovp_line_init(&ovp_rv_d3, RV_D3); ovp_line_init(&ovp_rv_ap4, RV_AP4); ovp_line_init(&ovp_rv_d4, RV_D4);
}
/* Our TONE grows brighter; OberVave's DAMP grows darker. All other mappings
 * use the upstream 0..127 coefficients. Snapshot volatile values per audio block. */
static void ovp_plate_params(uint32_t size, uint32_t tone, uint32_t pre) {
    uint32_t d=size*127u/100u, damp=127u-tone*127u/100u;
    uint32_t x=damp*damp*2u<<15, root=0, bit=1u<<15;
    ovp_rev_decay=12000+(int32_t)((d*d*19500u)>>14);
    for(;bit;bit>>=1)if((root+bit)*(root+bit)<=x)root+=bit;
    ovp_rev_damp_k=32768-(int32_t)root;
    ovp_rev_pre_n=1u+(pre*127u/100u)*41u;
}
static int32_t ovp_plate_decay, ovp_plate_damp;
static uint32_t ovp_plate_pre;
static void ovp_plate_block(uint32_t n, int32_t mix) {
    ovp_plate_decay=ovp_rev_decay;ovp_plate_damp=ovp_rev_damp_k;ovp_plate_pre=ovp_rev_pre_n;
    if(mix)ovp_rev_active=24u*FS;
    else ovp_rev_active=ovp_rev_active>n?ovp_rev_active-n:0;
}
static inline void ovp_rev_tick(int32_t in)
{
    int32_t x, a, b, e1 = ovp_tri_q(ovp_rv_ph1), e2 = ovp_tri_q(ovp_rv_ph2), dec = ovp_plate_decay, dk = ovp_plate_damp;
    ovp_rv_ph1 += 97391u * REV_DIV;                 /* ~1.0 Hz and ~0.7 Hz: the tank's sway against metallic rings */
    ovp_rv_ph2 += 68174u * REV_DIV;
    ovp_line_write(&ovp_rv_pre, in);
    x = ovp_line_read(&ovp_rv_pre, ovp_plate_pre);
    ovp_rv_lp_in += ((x - ovp_rv_lp_in) * RV_BW) >> 15;
    x = ovp_ap_step(&ovp_rv_in[0], ovp_rv_lp_in, 24576);
    x = ovp_ap_step(&ovp_rv_in[1], x, 24576);
    x = ovp_ap_step(&ovp_rv_in[2], x, 20480);
    x = ovp_ap_step(&ovp_rv_in[3], x, 20480);
    /* left half, fed by the right's end */
    a = x + ((ovp_line_read(&ovp_rv_d4, RV_D4) * dec) >> 15);
    a = ovp_ap_mod(&ovp_rv_ap1, a, -22938, RV_AP1, e1);
    ovp_line_write(&ovp_rv_d1, a);
    b = ovp_line_read(&ovp_rv_d1, RV_D1);
    ovp_rv_lpl += ((b - ovp_rv_lpl) * dk) >> 15;
    b = ovp_ap_step(&ovp_rv_ap2, (ovp_rv_lpl * dec) >> 15, 16384);
    ovp_line_write(&ovp_rv_d2, b);
    /* right half, fed by the left's end */
    a = x + ((ovp_line_read(&ovp_rv_d2, RV_D2) * dec) >> 15);
    a = ovp_ap_mod(&ovp_rv_ap3, a, -22938, RV_AP3, e2);
    ovp_line_write(&ovp_rv_d3, a);
    b = ovp_line_read(&ovp_rv_d3, RV_D3);
    ovp_rv_lpr += ((b - ovp_rv_lpr) * dk) >> 15;
    b = ovp_ap_step(&ovp_rv_ap4, (ovp_rv_lpr * dec) >> 15, 16384);
    ovp_line_write(&ovp_rv_d4, b);
    /* the output taps (the paper's), x 0.6 */
    a = ovp_line_read(&ovp_rv_d3, RVL(266)) + ovp_line_read(&ovp_rv_d3, RVL(2974)) - ovp_line_read(&ovp_rv_ap4, RVL(1913)) + ovp_line_read(&ovp_rv_d4, RVL(1996))
      - ovp_line_read(&ovp_rv_d1, RVL(1990)) - ovp_line_read(&ovp_rv_ap2, RVL(187)) - ovp_line_read(&ovp_rv_d2, RVL(1066));
    b = ovp_line_read(&ovp_rv_d1, RVL(353)) + ovp_line_read(&ovp_rv_d1, RVL(3627)) - ovp_line_read(&ovp_rv_ap2, RVL(1228)) + ovp_line_read(&ovp_rv_d2, RVL(2673))
      - ovp_line_read(&ovp_rv_d3, RVL(2111)) - ovp_line_read(&ovp_rv_ap4, RVL(335)) - ovp_line_read(&ovp_rv_d4, RVL(121));
    ovp_rv_yl = (int32_t)(((int64_t)a * 19661) >> 15);
    ovp_rv_yr = (int32_t)(((int64_t)b * 19661) >> 15);
}

