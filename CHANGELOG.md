# SMKTRON — Changelog

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

## 0.3.0-mm1h · 2026-11-14 · Previous official / Oficial anterior

### Português

MM1h (_462) aprovada por Marcus nos testes físicos e promovida sem recompilar: exatamente o pacote testado.

- Teclas e sustain atendidos durante a transferência da tela: melhora de latência e timing dos acordes no modo MIDI, incluindo as extremidades.
- Pitch bend e modulação também atendidos durante a atualização do display, reduzindo os degraus causados pela espera da interface. Pitch bend de 14 bits e retorno ao centro preservados.
- Canais MIDI confirmados: GLOBE → K3 CH escolhe o canal principal das teclas; PARA → K3 CH regula o controle selecionado.
- Sons e comportamento do synth interno, samples, mapas MIDI, cenas e sequenciador preservados. A melhoria de atendimento durante a tela aplica-se ao controlador MIDI.
- Instalador oficial, simulador, fontes e manual atualizados; atualização direta de MM1f, MM1g e MM1h. MM1f permanece disponível para retorno.

### English

Hardware-approved MM1h (_462), promoted with exactly the tested firmware bytes. Keys and sustain are serviced during screen transfers, improving external MIDI chord timing across the keyboard. Pitch bend and modulation are serviced during redraws too, removing the screen-induced gaps while preserving 14-bit bend and centre return. GLOBE → K3 CH sets the keyboard's main channel; PARA → K3 CH sets the selected control's channel. Internal synth sound/behaviour, samples, maps, scenes and sequencer are retained. Updated official installer, simulator, source and manual; direct update from MM1f/MM1g/MM1h. MM1f remains available for rollback.

## 0.3.0-mm1f · 2026-11-14 · Previous official / Oficial anterior

### Português

Versão aprovada em testes físicos, promovida sem recompilar ou alterar os bytes do firmware MM1f (_460).

- MIDI MODE exclusivo: instrumento sem MIDI musical; controlador sem áudio interno. Identificação/atualização USB disponível nos dois modos.
- USB-MIDI e TRS OUT, quatro mapas editáveis no próprio teclado, encoders ABS acelerados ou REL, faders e pads A/B, canais, CCs/notas, cores, velocity, pitch bend, modulação, sustain com polaridade automática e Panic.
- Telas de entrada, GLOBAL e edição ampliadas; quatro blocos de knobs e suporte a Zoom. Diagnóstico técnico retirado da tela diária.
- TRS: acessos UART alinhados ao SDK e reinicialização na entrada do controlador; USB cheio não bloqueia o TRS. Saída física aprovada.
- K5: leitura exclusiva no controlador, evitando que a rotina de volume do synth consuma passos. Volume do synth preservado ao voltar.
- Mapas MIDI com armazenamento separado e duas cópias CRC. Cenas, acordes, boot e todos os sons/efeitos/recursos da GR3 preservados. Sem Bluetooth.
- Site, simulador, instalador, manual e fontes atualizados no GitHub. Atualização direta de GR3 e versões SMKTRON reconhecidas; primeira instalação via V016 oficial. Firmware e fonte disponíveis na Release.

### English

Hardware-approved MM1f (_460) promoted with identical firmware bytes. Adds exclusive MIDI controller mode with USB/TRS OUT, four on-device editable maps, ABS/REL encoders, A/B controls/pads, selectable colours, velocity, bend, modulation, auto-polarity sustain and Panic. Larger entry/GLOBAL/editor screens with Zoom. Fixes TRS setup and USB/TRS queue independence, plus K5 events being consumed by synth volume while in controller mode. Separate CRC-backed MIDI-map storage. All GR3 sounds, saves and performance features retained. No Bluetooth. Website, simulator, installer, manual and source updated on GitHub; direct upgrade from recognized SMKTRON versions.

### Known behaviour / Comportamento conhecido

- Synth mode does not play incoming musical MIDI. MIDI MODE is a controller, with no MIDI Thru or DAW state feedback.
- Browser maps do not automatically transfer to hardware; musical-state backups exclude MIDI maps.
- USB video recording noise on some iPhone/app combinations remains a known inherited limitation; this release does not change USB audio.

## 0.3.0-gr3 · Previous official / Oficial anterior

Four tape banks; layer, scenes/favourites, chord pads, sequencer and GRAIN; Light/Dark/Zoom. Available as a prior GitHub Release. GRAIN replaces the arpeggiator; the physical ARP button opens GRAIN.
