# SMKTRON — Mellotron + MIDI controller for SMK-37 Pro

**Official / Oficial: 0.3.0-mm1f · SMK-37 Pro_460.** Approved on hardware by Marcus Padrini. The published firmware is byte-for-byte the approved MM1f test package.

[Website / Site](https://4t59m4f784-creator.github.io/SMKTRON/) · [Installer / Instalador](https://4t59m4f784-creator.github.io/SMKTRON/install/) · [Simulator / Simulador](https://4t59m4f784-creator.github.io/SMKTRON/simulator.html) · [Manual](https://4t59m4f784-creator.github.io/SMKTRON/manual/) · [Changelog](CHANGELOG.md) · [Releases](https://github.com/4t59m4f784-creator/SMKTRON/releases)

## Português

Instrumento Mellotron/lo-fi com FLUTE, CHOIR, CELLO e VIOLINS; layer, cenas/favoritos, acordes nos pads, sequenciador, GRAIN, tape wear, vibrato, Leslie, echo e reverb. Light/Dark/Zoom. Todos os sons e recursos da GR3 foram preservados.

**MIDI MODE independente:** synth não transmite nem interpreta MIDI musical; controlador silencia áudio interno e envia MIDI USB/TRS. Quatro mapas editáveis no teclado, ABS/REL, pads com cores, velocity, pitch bend, modulação, sustain e Panic. Sem Bluetooth.

- GLOBE → MIDI MODE → K1 à direita: entrar. No controlador, GLOBE e K1 à esquerda: voltar ao synth.
- GLOBAL: K1 modo, K2 mapa, K3 canal principal, K4 ABS/REL.
- PARA: escolher/editar controle. K1 tipo, K2 CC/nota, K3 canal, K4 cor do pad; K5 seleciona controle, PATCH escolhe por toque, BT salva os quatro mapas.
- KNOB BANK/FADER BANK alternam os bancos correspondentes; ambos juntos alternam pads A/B. GLOBE + SEQ PLAY: Panic.
- K5 fica exclusivo do controlador no MIDI; no synth preserva seu comportamento de volume.

O teclado sempre inicia como synth. Mapas MIDI são separados de boot, cenas e acordes e não integram o backup musical. Consulte o [manual MIDI](https://4t59m4f784-creator.github.io/SMKTRON/manual/#midi-mode).

**Instalação:** somente SMK-37 Pro. Primeira instalação exige V016 oficial M-VAVE. Versões SMKTRON reconhecidas, incluindo GR3 (_453), atualizam diretamente. Chrome/Edge no computador, HTTPS, cabo USB de dados, demais aplicativos MIDI fechados. Selecione seu V016 original como arquivo de retorno; ele não é redistribuído. O instalador verifica modelo, SHA-256, CRC e identidade e pede confirmação antes de gravar. Confira **0.3.0-mm1f / _460**. Não desconecte durante a escrita. OCT− ao ligar abre a recuperação USB. Preserve seus dados antes de trocar firmware; o retorno a versões antigas pode não ler dados mais novos.

## English

Mellotron/lo-fi instrument: FLUTE, CHOIR, CELLO, VIOLINS, layers, scenes/favourites, chord pads, sequencer, GRAIN and tape/FX. Light/Dark/Zoom. All GR3 sounds and features retained.

Exclusive MIDI controller mode with USB/TRS OUT, four on-device editable maps, ABS/REL encoders, coloured pads, velocity, bend, modulation, sustain and Panic. Synth mode neither sends nor interprets musical MIDI. Controller mode silences audio. No Bluetooth.

GLOBE → MIDI MODE → K1 right enters. In the controller, GLOBE then K1 left returns. GLOBAL: K2 map, K3 main channel, K4 ABS/REL. PARA edits controls: K1 type, K2 CC/note, K3 channel, K4 pad colour, K5 selects control, PATCH picks by touch, BT saves. KNOB BANK/FADER BANK select A/B; both select pad bank. GLOBE + SEQ PLAY sends Panic. Maps have separate storage, excluded from musical backups. Boot always starts in synth mode. See the manual's MIDI chapter for an English guide.

SMK-37 Pro only. First install requires official V016; recognized SMKTRON versions including GR3 update directly. Use desktop Chrome/Edge, HTTPS and a data cable; close MIDI apps. Supply your original V016 for restore. SHA-256/CRC/model verification and explicit confirmation precede every write. Check **0.3.0-mm1f / _460**. Do not disconnect while writing. OCT− at boot opens USB recovery. Back up data; older firmware may not read newer saves.

## Sources and build

- `firmware/`: complete frozen MM1f engine, source and corresponding build tools. `cd firmware && BOARD=smk37 MODEL=pro sh build.sh --release 0.3.0-mm1f`, with `JIELI_TOOLCHAIN` and `AC79_SDK` as described in its documentation. Identity _460 is pinned in its build tools.
- `experiments/midi/`: integration overlay and regression tests. `python experiments/midi/build.py` generates the simulator; add `--native` for the hardware package. Set `ZIG` (Zig 0.13) and the native SDK/toolchain variables. The overlay uses the GR3 base under `experiments/appearance/engine` and `experiments/chords`.
- `web/`: GitHub Pages website, simulator, installer, manual and source archive. `manual/` contains editable documentation sources; `python tools/build_manual.py` regenerates the manual.
- Exact package SHA-256: `9249b40ad1c7c30f564790a6be90a5561ff0cee29ac5351c331f60a2fd3d7545`; 651266 bytes. App 599296/619520 bytes; 20224 free. Static RAM 33376 bytes. Sample tables unchanged.
- Verification: `npm test`; native K5 regression: `python experiments/midi/k5_test.py` after generating the engine. Physical USB/TRS, controller screens and K5 approved by the user. Browser checks do not substitute for electrical testing.

## Credits and licenses

See [CREDITOS.md](web/CREDITOS.md) and component notices in `firmware/`. Derived code GPL-3.0-only. FMEL-1 / AntonioAltea; SMK port / M. Geisler (genkigenki/makkuro); Dinghy / zvenson; Felucca / Leo Kuroshita, Hügelton; SLOOP / isod89; OberVave / zvenson and Jon Dattorro for reverb; MIDI controller conventions from Snoop37 and SMKSYNTH.

Samples: Bernie Kornowicz (Taijiguy), including MkII Violins. Separate noncommercial terms apply; they are not GPL or public domain. Do not sell products containing these recordings. Distributed free of charge. Original notices and licenses retained.

Concepção, direção musical e testes / concept, musical direction and tests: **Marcus Padrini**. Adaptations, UI, controls, documentation and web tools developed with ChatGPT/Codex (OpenAI) under his direction. Independent project, without M-VAVE or upstream endorsement.
