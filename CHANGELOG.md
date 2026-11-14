# SMKTRON — Changelog

## 0.3.0-mm1f · 2026-11-14 · Official / Oficial

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
