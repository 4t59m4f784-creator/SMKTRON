## 0.3.0-ml1 · 2026-11-18 · Official / Oficial

### Português

ML1 (_463) aprovada por Marcus no instrumento e promovida com os mesmos bytes do pacote testado.

- MIDI Layers: banco B dos pads torna-se seletor dos canais 1–16. Toque sozinho escolhe um canal; segure e adicione pads para combinar canais; solte para manter o layer. Um novo toque sozinho substitui o conjunto.
- Notas já tocadas mantêm seus destinos e recebem Note Off nos canais de origem. Sustain lembra destinos anteriores até soltar o pedal. Pitch bend de 14 bits e modulação acompanham o conjunto e sincronizam os canais adicionados.
- Banco A, knobs, faders e demais controles preservam seus mapas e canais próprios/MAIN. Seletores B não enviam notas/CCs de pad nem são remapeáveis; os antigos dados B permanecem guardados no formato compatível dos mapas.
- Seleção temporária: GLOBAL/K3, troca de mapa ou nova entrada no MIDI volta a um canal principal. Panic e saída do controlador liberam todos os canais.
- Timing e bend contínuo da MM1h preservados. Sons, efeitos e comportamento do synth interno, cenas, acordes e sequenciador preservados.
- Site, instalador oficial, simulador, fontes e manual PT/EN atualizados. Simulador mostra canais/layers e oferece Shift+clique para adicionar canais no computador. MM1h disponível para retorno.

### English

Hardware-approved ML1 (_463), promoted with exactly the tested firmware bytes. Pad bank B selects channels 1–16: tap for solo, hold and add pads for a latched layer; a new solo tap replaces the selection. Note Off returns to original destinations; sustain remembers old channels until released; 14-bit bend and modulation follow and prime layer channels. Bank A and ordinary controls retain mapped channels/MAIN. B selectors do not send mapped pad notes/CCs and are not editable; saved maps remain compatible. The selection is temporary: GLOBAL/K3, map change or re-entry selects one main channel. Panic/exit release all destinations. MM1h timing and internal synth behaviour are retained. Website, official installer, simulator, source and PT/EN MIDI manual updated; desktop simulator adds channels with Shift+click. MM1h remains available for rollback.

## Files / Arquivos

Firmware `.fwsc`; full repository source `.zip`; `RELEASE.json` with identity, hash and source commit; `SHA256SUMS.txt`; credits and changelog.

Derived code GPL-3.0-only. FMEL-1 / AntonioAltea; SMK port / M. Geisler (genkigenki/makkuro); Dinghy / zvenson; Felucca / Leo Kuroshita, Hügelton; SLOOP / isod89; OberVave / zvenson and Jon Dattorro for reverb. Controller work derived from Snoop37 and SMKSYNTH. Original notices retained.

Samples: Bernie Kornowicz (Taijiguy), separate noncommercial terms, not GPL or public domain. Do not sell products containing these recordings. Distributed free of charge. Concept, musical direction and tests: Marcus Padrini. Adaptations, interface, documentation and tools developed with ChatGPT/Codex (OpenAI) under his direction. Independent project, no upstream or M-VAVE endorsement.

[Website](https://4t59m4f784-creator.github.io/SMKTRON/) · [Simulator](https://4t59m4f784-creator.github.io/SMKTRON/simulator.html) · [MIDI manual](https://4t59m4f784-creator.github.io/SMKTRON/manual/#midi-mode)
