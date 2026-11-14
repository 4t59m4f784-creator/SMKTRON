/* SPDX-License-Identifier: GPL-3.0-only */
/* MEL's arpeggiator and step sequencer, on one clock (96 ticks a beat, (uint32_t)PV(P_BPM)). Main loop only; included by
 * mel.c after the notes (src_set) and the settings.
 *
 * ARP: while it is on, the keys (and MIDI, HOLD, the pedal) do not sound themselves: they make the arpeggio.
 *   MODE UP DOWN UP+DN RANDOM PLAYED, RATE 1/4 .. 1/32, OCT 1..3 octaves, GATE 10..100 %.
 * SEQ: 16 steps of 1/16, up to 4 notes each (a chord: it is a Mellotron), LENGTH 1..16, GATE, SWING (the off-beat
 *   sixteenths late: 0 straight .. 100 a triplet feel).
 *   PLAY starts / stops. REC, stopped: step recording (play a note or a chord, let go: the next step; SEL: a rest).
 *   REC while playing: what you play goes into the step it falls on. SEL held 1 s: the pattern is cleared. */

enum { S_ARP = 16, S_SEQ = 32 };
#define S_IN (S_KEY | S_MIDI | S_LATCH | S_PED)
static const uint8_t RATE_TICKS[6] = {96, 48, 32, 24, 16, 12};

enum {arp_on=0,arp_cur=255,arp_n=0,arp_pos_shown=0};
static uint8_t seq_play,seq_rec;
static const uint8_t arp_list[1]={0};
static uint32_t seq_step, seq_tick, seq_cur, seq_gate_left;
static uint8_t seq_sounding[4];
static uint32_t clk_acc, clk_last;
static uint8_t chord_open;                      /* step recording: a chord is being played into seq_cur */

/* ---- input: to the voices, or to the arpeggio */
static void in_set(uint32_t note,uint32_t set_b,uint32_t clr_b){if(note<128u)src_set(note,set_b,clr_b);}
static uint8_t in_get(uint32_t note){return note<128u?(uint8_t)(src[note]&S_IN):0;}
static void in_clear(uint32_t bits){uint32_t n;for(n=0;n<128u;n++)if(in_get(n)&bits)in_set(n,0,bits);}
static void arp_release(void){}
static void arp_enable(int on){(void)on;}
/* ---- the sequencer */
static void seq_release(void)
{
    uint32_t k;
    for (k = 0; k < 4u; k++)
        if (seq_sounding[k]) {
            src_set(seq_sounding[k], 0, S_SEQ);
            seq_sounding[k] = 0;
        }
}
static void seq_trigger(void)
{
    uint32_t k;
    seq_release();
    for (k = 0; k < 4u; k++) {
        uint32_t nt = set.steps[seq_step][k];
        if (nt) {
            seq_sounding[k] = (uint8_t)nt;
            src_set(nt, S_SEQ, 0);
        }
    }
    seq_gate_left = 24u * (uint32_t)PV(P_SGATE) / 100u;
    if (!seq_gate_left) seq_gate_left = 1;
    if (page == PG_SEQ) redraw |= 2u;
}
static void seq_start(int on)
{
    if (on && !seq_play) {
        seq_play = 1;
        seq_step = 0;
        seq_tick = 0;
        clk_acc = 0;
        seq_trigger();
    } else if (!on && seq_play) {
        seq_play = 0;
        seq_release();
    }
    redraw |= 3u;
}
static int step_add(uint32_t st, uint32_t note)
{
    uint32_t k;
    for (k = 0; k < 4u; k++)
        if (set.steps[st][k] == note) return 1;
    for (k = 0; k < 4u; k++)
        if (!set.steps[st][k]) { set.steps[st][k] = (uint8_t)note; return 1; }
    return 0;
}
static void rec_note(uint32_t note)            /* a note played while REC is on */
{
    if (!seq_rec || note > 127u) return;
    if (seq_play) {                             /* into the step it falls on */
        uint32_t st = seq_tick < 12u ? seq_step : (seq_step + 1u) % (uint32_t)PV(P_SLEN);
        step_add(st, note);
    } else {                                    /* step recording: the first note of a chord clears the step */
        if (!chord_open) {
            uint32_t k;
            for (k = 0; k < 4u; k++) set.steps[seq_cur][k] = 0;
            chord_open = 1;
        }
        step_add(seq_cur, note);
    }
    changed(99);
    redraw |= 2u;
}
static void rec_all_up(void)                   /* every key let go: step recording goes on to the next step */
{
    if (chord_open) {
        chord_open = 0;
        seq_cur = (seq_cur + 1u) % (uint32_t)PV(P_SLEN);
        redraw |= 10u;
    }
}

/* ---- the clock: 96 ticks a beat, run from the main loop on fm1_ms */
static void clock_run(void)
{
    uint32_t dt = fm1_ms - clk_last;
    clk_last = fm1_ms;
    if (dt > 50u) dt = 50u;
    if(!seq_play){clk_acc=0;return;}
    clk_acc += dt * (uint32_t)PV(P_BPM) * 96u;
    while (clk_acc >= 60000u) {                 /* one tick */
        clk_acc -= 60000u;
        if (seq_play) {
            uint32_t sw = (uint32_t)PV(P_SWING) * 8u / 100u, len = seq_step & 1u ? 24u - sw : 24u + sw;
            if (seq_gate_left && !--seq_gate_left) seq_release();
            if (++seq_tick >= len) {
                seq_tick = 0;
                seq_step = (seq_step + 1u) % (uint32_t)PV(P_SLEN);
                seq_trigger();
            }
        }
    }
}
