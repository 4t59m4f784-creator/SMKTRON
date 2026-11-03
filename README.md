# SMKTRON — Mellotron for SMK-37 Pro

Mellotron / lo-fi instrument firmware for M-VAVE SMK-37 Pro. FLUTE, CHOIR, CELLO and VIOLINS; layer, chord pads, sequencer, GRAIN, tape wear, vibrato, Leslie, echo and reverb. Light / Dark / Zoom displays.

## Site · Simulador · Instalador / Website · Simulator · Installer

- [Site / Website](https://4t59m4f784-creator.github.io/SMKTRON/)
- [Simulador / Simulator](https://4t59m4f784-creator.github.io/SMKTRON/simulator.html)
- [Instalador / Installer](https://4t59m4f784-creator.github.io/SMKTRON/install/)

Site, simulador e instalador estão disponíveis em português e inglês. / The website, simulator and installer are available in Portuguese and English.

## Origem e proposta / Origins and purpose

O SMKTRON surgiu do excelente FMEL-1 para o FM-1, que consegue soar muito bem em um dispositivo com tão pouco armazenamento. A proposta do SMKTRON foi ajustar alguns samples e, principalmente, adaptar a interface e os controles para um instrumento focado em performance no M-VAVE SMK-37 Pro.

SMKTRON grew out of the excellent FMEL-1 for the FM-1, which achieves an impressive sound on a device with very little storage. SMKTRON's goal was to adjust selected samples and, above all, adapt the interface and controls into a performance-focused instrument for the M-VAVE SMK-37 Pro.

Concepção, direção musical e testes: Marcus Padrini. Desenvolvimento das adaptações, interface, controles, documentação e ferramentas web com participação do ChatGPT/Codex (OpenAI). Os projetos e autores originais permanecem creditados por suas contribuições.

Concept, musical direction and testing: Marcus Padrini. Adaptations, interface, controls, documentation and web tools developed with the participation of ChatGPT/Codex (OpenAI). The original projects and authors retain credit for their contributions.

## Install / Instalação

First installation requires official M-VAVE **SMK-37 Pro V016**, identity `SMK-37 Pro_016`. Use M-UPGRADE to return to V016 first. Back up or note your settings before changing firmware. Other models and firmware origins are rejected. Use Chrome/Edge on desktop, HTTPS, USB data cable; close MIDI applications. Select your original V016 file, connect, prepare and confirm. Never disconnect during writing. Check `0.3.0-gr3` at boot.

A primeira instalação exige V016 oficial. O instalador verifica SHA-256, CRCs e modelo. O arquivo V016 é selecionado localmente, não é redistribuído neste projeto. Não existe instalação automática ao abrir a página. Registros novos são v7; a versão AT2 não os lê.

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

The public site, simulator and installer are hosted on GitHub Pages at the links above. The Pages workflow publishes `web/` when its files change on `main`. Firmware writing always requires explicit confirmation in the installer.

## Credits and licenses

See `web/CREDITOS.md` and licenses in the frozen engine. Derived code: GPL-3.0-only. Upstream: FMEL-1 / AntonioAltea, SMK port genkigenki, Dinghy / zvenson, Felucca / Hügelton, SLOOP / isod89; reverb adapted from OberVave (Dattorro).

Samples: Bernie Kornowicz (Taijiguy), with separate noncommercial terms; they are not GPL or public domain. Do not sell products containing these recordings. This project is distributed free of charge. Concept, musical direction and testing: Marcus Padrini. Adaptations, interface, controls, documentation and web tools developed with the participation of ChatGPT/Codex (OpenAI). The original projects and authors retain credit for their contributions.
