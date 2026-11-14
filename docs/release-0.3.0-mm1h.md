## 0.3.0-mm1h · 2026-11-14 · Official / Oficial

### Português

MM1h (_462) aprovada por Marcus nos testes físicos e promovida sem recompilar: exatamente o pacote testado.

- Teclas e sustain atendidos durante a transferência da tela: melhora de latência e timing dos acordes no modo MIDI, incluindo as extremidades.
- Pitch bend e modulação também atendidos durante a atualização do display, reduzindo os degraus causados pela espera da interface. Pitch bend de 14 bits e retorno ao centro preservados.
- Canais MIDI confirmados: GLOBE → K3 CH escolhe o canal principal das teclas; PARA → K3 CH regula o controle selecionado.
- Sons e comportamento do synth interno, samples, mapas MIDI, cenas e sequenciador preservados. A melhoria de atendimento durante a tela aplica-se ao controlador MIDI.
- Instalador oficial, simulador, fontes e manual atualizados; atualização direta de MM1f, MM1g e MM1h. MM1f permanece disponível para retorno.

### English

Hardware-approved MM1h (_462), promoted with exactly the tested firmware bytes. Keys and sustain are serviced during screen transfers, improving external MIDI chord timing across the keyboard. Pitch bend and modulation are serviced during redraws too, removing the screen-induced gaps while preserving 14-bit bend and centre return. GLOBE → K3 CH sets the keyboard's main channel; PARA → K3 CH sets the selected control's channel. Internal synth sound/behaviour, samples, maps, scenes and sequencer are retained. Updated official installer, simulator, source and manual; direct update from MM1f/MM1g/MM1h. MM1f remains available for rollback.

## Files / Arquivos

Firmware `.fwsc`; full repository source `.zip`; `RELEASE.json` with identity, hash and source commit; `SHA256SUMS.txt`; credits and changelog.

Derived code GPL-3.0-only. FMEL-1 / AntonioAltea; SMK port / M. Geisler (genkigenki/makkuro); Dinghy / zvenson; Felucca / Leo Kuroshita, Hügelton; SLOOP / isod89; OberVave / zvenson and Jon Dattorro for reverb. Controller work derived from Snoop37 and SMKSYNTH. Original notices retained.

Samples: Bernie Kornowicz (Taijiguy), separate noncommercial terms, not GPL or public domain. Do not sell products containing these recordings. Distributed free of charge. Concept, musical direction and tests: Marcus Padrini. Adaptations, interface, documentation and tools developed with ChatGPT/Codex (OpenAI) under his direction. Independent project, no upstream or M-VAVE endorsement.

[Website](https://4t59m4f784-creator.github.io/SMKTRON/) · [Simulator](https://4t59m4f784-creator.github.io/SMKTRON/simulator.html) · [MIDI manual](https://4t59m4f784-creator.github.io/SMKTRON/manual/#midi-mode)
