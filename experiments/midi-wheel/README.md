# MM1h / SMK-37 Pro_462 - continuous external pitch bend

Experimental, awaiting Marcus's physical comparison. Includes the MM1g keyboard latency fix, confirmed by Marcus to improve external MIDI chord timing.

MM1g serviced keys and sustain during LCD SPI waits, but wheel polling remained in the main loop. The SMKSYNTH implementation also polls wheels from its LCD idle callback. MM1h follows that scheduling: pitch bend and modulation are read at the same millisecond opportunities as the keys, before queue service. This prevents intermediate wheel positions from waiting behind large screen transfers. No change to bend calibration, ADC filtering, channel routing or 14-bit LSB/MSB encoding. Wheel selection in the editor stays exclusively on the main loop.

## Validation

`python3 tests/midi-wheel-lcd-test.py` executes the actual wheel function, queue service and HAL wait against a simulated moving ADC. It reproduces the MM1g starvation, then verifies multiple monotonic intermediate bend values during one transfer, nonzero low seven bits, selected channel, exact return to centre, no duplicate main-loop send, CC remapping, disabled mapping and editor isolation. This is a host scheduling regression, not a physical latency measurement.

The MM1g keyboard regression and 13 WASM/installer scenarios pass. Audio output, scenes and sequence remain identical to MM1f in the comparison suite. Package identity, CRC, flash region table and loader marker checks pass. Hardware validation is still required.

Native app 599,856/619,520 bytes; 19,664 free; static RAM 33,392 bytes. Firmware 651,266 bytes. SHA-256: `0c3af65e1c7e2eb619af42b301ce87935ba991a8a682fdf0def84e4554b64b75`.

Build the current frozen sources with `python3 experiments/midi-latency/build.py`; add `--web` with `ZIG` set to Zig 0.13.0 for WASM. Native dependencies: `JIELI_TOOLCHAIN` and `AC79_SDK` as documented in `firmware/build.sh`.

Test installer: `/SMKTRON/install-mm1h/`. MM1g (_461) can update directly. Official rollback: `/SMKTRON/rollback-mm1f/`, accepting _462. Official MM1f and its installer remain unchanged. Source branch: `experiment/mm1h-midi-bend`.
