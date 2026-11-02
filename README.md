# SMKTRON GR3 — SMK-37 Pro

Mellotron / lo-fi instrument firmware for M-VAVE SMK-37 Pro. FLUTE, CHOIR, CELLO and VIOLINS; layer, chord pads, sequencer, GRAIN, tape wear, vibrato, Leslie, echo and reverb. Light / Dark / Zoom displays.

## Install / Instalação

First installation requires official M-VAVE **SMK-37 Pro V016**, identity `SMK-37 Pro_016`. Use M-UPGRADE to return to V016 first. Back up or note your settings before changing firmware. Other models and firmware origins are rejected. Use Chrome/Edge on desktop, HTTPS, USB data cable; close MIDI applications. Select your original V016 file, connect, prepare and confirm. Never disconnect during writing. Check `0.3.0-gr3` at boot.

A primeira instalação exige V016 oficial. O instalador verifica SHA-256, CRCs e modelo. O arquivo V016 é selecionado localmente, não é redistribuído neste projeto. Não existe instalação automática ao abrir a página. Registros novos são v7; a versão AT2 não os lê.

Official simulator / instalador: https://smktron-rf1.padrini.chatgpt.site/

## Controls / Controles

- K1–K4: page parameters; K5 scene volume. Global master is separate.
- ARP button opens GRAIN; K6 selects FREE / BRUMA / PULSOS / FRAGMENTOS. Performance pad 10 toggles GRAIN.
- GRAIN: SIZE / SPREAD / DENSITY / MIX. Saved with boot settings and favorites.
- Modulation wheel opens VIBRATO and controls mix up to MAX MIX.
- KNOB BANK: Performance → Favorites → Chord Pads.
- Chord top 8 pads: chords; bottom: REC, HOLD, FLUTE, CHOIR, CELLO, VIOLINS, STOP, EXIT.
- Sequencer remains available; arpeggiator is removed.

## Source / Build

Complete frozen engine: `experiments/appearance/engine`. Overlay build: `python experiments/chords/build.py --grain3` (Zig 0.13); native: add `--native` and provide `JIELI_TOOLCHAIN` and `AC79_SDK`. See build scripts for tool paths. Preserve original four sample banks and license files. Firmware binary: `web/install/firmware/`. App 586032/619520 bytes, 33488 free; static RAM 31744 bytes. Native build and simulator checks passed; no new physical test is claimed for the V016-only installer wrapper.

## GitHub Pages

Repository: https://github.com/4t59m4f784-creator/SMKTRON. Enable Pages with GitHub Actions, then run the Pages workflow. It publishes `web/`; firmware writing still requires explicit user confirmation. The hosted official installer remains available at the link above.

## Credits and licenses

See `web/CREDITOS.md` and licenses in the frozen engine. Derived code: GPL-3.0-only. Upstream: FMEL-1 / AntonioAltea, SMK port genkigenki, Dinghy / zvenson, Felucca / Hügelton, SLOOP / isod89; reverb adapted from OberVave (Dattorro).

Samples: Bernie Kornowicz (Taijiguy), with separate noncommercial terms; they are not GPL or public domain. Do not sell products containing these recordings. This project is distributed free of charge. Marcus Padrini: musical direction and testing; adaptations with ChatGPT/Codex.
