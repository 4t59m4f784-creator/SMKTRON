# MM1g / _461 - experimental MIDI latency correction

Status: native build and host tests passed; **physical validation pending**. MM1f / _460 remains the official release.

The MM1f MIDI loop processes the complete keyboard mask, sends queued messages, then redraws the display. Its SPI wait does not yield, so a note arriving after the first poll can wait for a large LCD transfer. SMKSYNTH/OberVave uses `LCD_IDLE` to keep notes moving while SPI is busy, including its separate outer-key queue.

MM1g adapts that cooperative mechanism to SMKTRON's existing complete 37-key MIDI mask. The bounded HAL SPI wait calls a notes-only callback: keyboard events and pedal are serviced at millisecond opportunities, and queued MIDI messages can drain between them. It does not call navigation, drawing, map saving or mode changes recursively. Entry/exit gating and a recursion guard apply. The normal loop and callback share the same note handler, avoiding duplicate events. A brief interrupt mask protects the 64-bit key snapshot on the 32-bit CPU. All original velocity and note ownership logic is preserved.

Scope: controller keyboard and sustain during LCD/LED SPI waits. This does not make TRS MIDI physically simultaneous, change its baud rate, or certify an end-to-end latency number. The synth path is unchanged. Pads, wheels and navigation continue through the normal panel loop. Measurements in the native test are simulated scheduling bounds, not hardware measurements.

## Build

Set `JIELI_TOOLCHAIN` and `AC79_SDK` as in `firmware/build.sh`. The three SDK packaging assets match the original MM1f SDK_SHA256 values exactly. Build with:

```
python3 experiments/midi-latency/build.py
ZIG=/path/to/zig-0.13.0 python3 experiments/midi-latency/build.py --web
python3 tests/midi-lcd-latency-test.py
node --test tests/mm1g-regression.mjs tests/mm1g-installer.test.mjs
```

Use this builder with the frozen `firmware/` on this branch. The historical MM1f generator remains unchanged and is not the MM1g builder.

## Verification

- Simulated LCD wait: notes at the first, middle and last key are sent before DMA finishes; velocity and releases preserved; no duplicate normal-poll events.
- Edit/entry/exit blocking and 80-message Panic service checked against the actual note handler and queue service.
- 11 WASM regression scenarios: 407,040 compared output values remain bit-identical to MM1f across tapes, layer and GRAIN; sequence, scenes and boot storage match; mappings, banks, save failure, sustain polarities and mode return pass.
- Native package: CRC, identity, region table and loader marker verified; build checks confirm app size, RAM, protected register access and RAM routines.
- USB/TRS router regression passes; separate experimental installer validates SHA-256, model and allowed source versions. Rollback page accepts _461 and reinstalls the unchanged _460 official package.

App: 599,736 of 619,520 bytes; 19,784 free. Static RAM: 33,392 bytes. Package: 651,266 bytes. SHA-256: `85bfdd6616faa746ed41949cfb695210d42b2089ac5b1e01004f8dcb1ddc83c7`.

## Physical comparison

Install MM1g on SMK-37 Pro. Compare the same external patch, cable and MIDI path as MM1f/SMKSYNTH. Try repeated chords across the centre and both ends, soft and hard velocity, quick taps, long sustain and note releases, then return to synth. Listen for reduced staggering, no lost/held notes and preserved velocity. Public promotion requires Marcus's physical approval.

Test installer: `/SMKTRON/install-mm1g/`. Return to official MM1f: `/SMKTRON/rollback-mm1f/`. Standard installer and latest release remain MM1f.
