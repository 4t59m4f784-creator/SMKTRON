/* SPDX-License-Identifier: GPL-3.0-only
 * Read-only SMKTRON transport animation. One revolution in ~5 s (10 s HALF).
 * The three identical spokes repeat every 120 degrees: fast angles alias at
 * the LCD refresh rate, appearing stopped or reversed. Keep motion slow,
 * independent of voice amplitude and the audio wow/flutter oscillators.
 * Cap an unusually late poll at 250 ms (18 degrees) rather than catching up
 * with a large visual jump. No floating point or audio state is involved.
 */
static uint32_t smktron_reel_advance(uint32_t phase, uint32_t elapsed_ms, uint32_t half)
{
    if (elapsed_ms > 250u) elapsed_ms = 250u;
    return phase + elapsed_ms * (858993u >> (half != 0u));
}
