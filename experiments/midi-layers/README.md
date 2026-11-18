# SMKTRON ML1 - channel selector and instant layers

Official `0.3.0-ml1`, SMK-37 Pro_463. Physically approved by Marcus and promoted on 2026-11-18 with the exact tested firmware bytes. MM1h is available for rollback.

Ported from the existing SMKSYNTH ML1 implementation in commit `752c785bb4a3805add097dcde3b6e19000476f4a`, preserving source notices. SMKTRON retains its own interface, storage integration and cooperative LCD service from MM1h. Only controller behaviour changes; internal synth audio and state remain unchanged.

## Português

- KNOB BANK + FADER BANK juntos alternam banco A/B dos pads.
- A mantém seus pads configuráveis. B é fixo: pad 1 = canal 1, até pad 16 = canal 16.
- Um toque sozinho seleciona um canal. Segure um pad e adicione outros: os canais se somam. Solte todos para manter o layer. O próximo toque sozinho substitui o conjunto.
- A seleção acompanha o teclado mesmo de volta ao banco A. Notas já tocadas são liberadas nos canais em que começaram; não são redisparadas ao mudar a seleção.
- Sustain acompanha o conjunto e mantém histórico dos canais anteriores até soltar o pedal. Pitch bend e modulação seguem os canais selecionados, incluindo sincronização das posições atuais ao adicionar canais. Knobs, faders e controles comuns preservam seus canais próprios/MAIN.
- GLOBE/K3 ou troca de mapa volta a um único canal principal. Entrar novamente no controlador restaura o canal principal salvo. O conjunto de canais é temporário; os quatro mapas continuam no mesmo formato, sem apagar os antigos dados dos pads B.
- B não envia notas/CCs de pad e não é remapeável. PANIC e saída do MIDI liberam todos os canais. Seleções aparecem na tela e nos LEDs.

## English

Press KNOB BANK + FADER BANK together for pad bank A/B. Bank A remains mapped; B selects channels 1-16. Tap one pad for solo; hold it and add others for a layer. Release all pads to latch; a new solo tap replaces the selection. Returning to A does not change the selected keyboard channels. Held notes keep their original destinations until release. Sustain remembers old/new destinations; bend and modulation follow selected channels and prime newly selected ones. Ordinary controls keep their own channels. GLOBAL K3/map change or re-entering MIDI selects the saved main channel; layer selection is temporary. Saved maps remain byte-compatible. B emits no pad notes/CCs and is not editable. Panic and mode exit clear every destination.

## Validation and build

- `python3 tests/midi-layers-native.py`: reuses SMKSYNTH's ownership/backpressure assertions against actual SMKTRON core/pad/key functions. Covers same-frame routing, shared notes, sustain history, 37 x 16 notes, blocked output, retriggers, Panic and stored-map compatibility.
- `node --test tests/midi-layers.test.mjs`: full engine via WASM, actual physical-bank gesture, channel/layer latch, Note Off destinations, sustain, wheels, independent controls, all 592 notes, mode return and saved-map roundtrip.
- Existing keyboard/LCD and 14-bit wheel/LCD tests pass. Eleven existing WASM regression scenarios pass, including 407,040 output values bit-identical to the original MM1f synth. Official release tests remain unchanged and pass.
- Build: `python3 experiments/midi-layers/build.py`; `ZIG=/path/to/zig-0.13.0 python3 experiments/midi-layers/build.py --web`. Set `JIELI_TOOLCHAIN` and `AC79_SDK` as in `firmware/build.sh`. The ML1 builder uses the frozen `firmware/` tree, not the historical MM1f generator.

App 603,060/619,520 bytes (16,460 free); static RAM 33,712 bytes. Firmware 651,266 bytes. SHA-256: `8b975a1f38fb09727e697107603680325cb5bcca2a90807ef46d62e683a1b16f`.

USB/TRS have their physical transport limits: host tests do not establish electrical timing. Test on the same external instrument before official promotion.

Official installer: `/SMKTRON/install/`. Official simulator: `/SMKTRON/simulator.html`. Archived ML1 test paths remain available for interrupted-operation recovery. MM1h return: `/SMKTRON/rollback-mm1h/`.
