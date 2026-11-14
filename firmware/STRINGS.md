# Optional SMKTRON STRINGS bank — 0.2.0-rc3

Current selection: warm/Quente, −8 dB shelf at 1.8 kHz plus 4.2 kHz lowpass before ADPCM, per-root sustain RMS compensated. User selected after listening on 19/10/2026. Exact bytes copied from the approved audition header by tools/apply_strings_warm.py; zones, 8 roots, loop lengths, first 3 banks, DSP tables and flash/loader code unchanged. Version RC3 physical test pending. RC2 is frozen in release/0.2.0-rc2/source/ and its installer remains at /install-0.2.0-rc2/; native binaries/releases are preserved.

Build preview: ZIG=/path/to/zig sh build-strings-wasm.sh. Native: SMKTRON_STRINGS=1 BOARD=smk37 MODEL=pro sh engine/build.sh --release 0.2.0-rc3 with the pinned toolchain and SDK. The build remains opt-in; without SMKTRON_STRINGS the original VIBES header is selected. RC2 and RC3 share MIDI product Pro_452; the display shows the full release suffix and the installer pins a distinct SHA-256.

The source ZIP is not redistributed. The user states original Mellotron recordings with noncommercial use permitted; no independent licence document was included. Metadata and per-root hashes are in dist/strings-preview/bank.json. Sources/licensing of the original three banks remain unchanged.

The eight roots are A2, C#3, F3, A3, D4, G4, D#5 and F5. D#5 covers A#4–D5. The warm comparison preserves the selected D#5 body patch and uses periodic filter context to retain loop continuity. Firmware still loads saved parameters and enables effects normally at boot; the comparison page defaults to bypassing LOFI and FX only for audition.

Historical implementation notes follow:

# Optional SMKTRON STRINGS bank

This is an opt-in preview, version 0.2.0-rc2. The approved 0.1.1-rc1 and its web installer remain unchanged. Build only the browser preview with `ZIG=/path/to/zig sh build-strings-wasm.sh`. Native verification: `SMKTRON_STRINGS=1 BOARD=smk37 MODEL=pro sh engine/build.sh --release 0.2.0-rc2` with the existing JieLi toolchain and pinned AC79 SDK. Without SMKTRON_STRINGS the original four-bank header is used.

The first 36 zones (FLUTE/CHOIR/CELLO), all their ADPCM bytes and decoder checkpoints, and the original DSP tables are preserved exactly. Only bank 4 changes from VIBES to STRINGS, using 8 roots A2/C#3/F3/A3/D4/G4/C5/F5. STRINGS has no VIBES decay metadata. Existing memory/loader/flash map code is unchanged.

The source ZIP String Section.zip was supplied by the user, who states that the recordings come from an original Mellotron and permit noncommercial use. The ZIP contains no licence or attribution document. This is a user-provided permission statement, not an independently verified public-domain designation. It does not change the licensing of the code or the three original Taijiguy banks. Source SHA, per-root ADPCM hashes and reconstruction metadata are in dist/strings-preview/bank.json. The uploaded source ZIP is not redistributed.

`tools/build_strings_bank.py INPUT.zip STUDY_DIRECTORY` reads the selected audition's exact decoded waveform to recover the rounded gain precision and verifies each sample against that audition before writing the optional header. No fresh loop search or root reassignment is used to change the user's choice: each selected duration is reconstructed and checked against the recorded source-region choice. The preview waveform is preserved at its 16-bit export precision, including the decoder's checkpoint-predictor return on wrap. The real firmware then uses its original Hermite interpolation, envelope, tuning and effects.

This stage enables audition in the actual engine. No physical installation is authorized or performed by this preview. Native builds verify capacity; hardware stability and musical assessment remain for the following stage.

The user selected D#5 from the targeted body comparison. Only zone 42 (MIDI 70..74) now uses that source recording (root 75, transpose −5..−1); all other samples/metadata remain identical to the accepted 8-root audition. The RC1 reference header is frozen under release/0.2.0-rc1-preview/source/ for rebuilding the historical A/B engines. RC2's hardware installer is separate at /install-strings/; hardware validation remains pending.
