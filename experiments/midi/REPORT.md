# SMKTRON MM1f — MIDI MODE / K5

Protótipo experimental para SMK-37 Pro. Base GR3, controlador portado da MM1a do SMKSYNTH (19639), fonte Site commit 3d2683a426b0bf0b8704fc127f18375c509efddd. Identidade atual SMK-37 Pro_460. USB/TRS aprovados na MM1d; MM1e amplia a interface. A oficial GR3 permanece inalterada.

## Português

GLOBAL (GLOBE): SETUP → SYSTEM → PAD CHORDS → MIDI MODE. Na página MIDI MODE, K1 para a direita entra no controlador. No controlador, GLOBE abre GLOBAL / CONTROLLER; K1 para a esquerda retorna ao instrumento. O simulador também oferece botões diretos SMKTRON / MIDI MODE.

O modo instrumento não transmite nem interpreta MIDI musical. A comunicação técnica de identificação e atualização USB/SysEx permanece disponível. O modo controlador silencia o áudio e para sequenciador, HOLD, acordes e gravação. MIDI recebido não toca o synth e não é retransmitido. Ao voltar, a execução permanece parada; solte teclas, pads e pedal para rearmar. Sempre inicia em SMKTRON ao ligar.

Quatro mapas independentes. GLOBAL do controlador: K2 mapa, K3 canal principal, K4 ABS / REL. KNOB BANK: encoders A/B; FADER BANK: faders A/B; ambos juntos: pads A/B. Encoders ABS com aceleração 1/2/4/8, inversão fina; REL 64 ± passos. Teclado com velocity e oitavas, pitch de 14 bits, modulação, sustain físico com polaridade calibrada ao ligar (conecte solto), Panic GLOBE + PLAY.

PARA abre edição: toque/mova o controle para selecionar; o primeiro movimento apenas seleciona. K1 tipo, K2 CC/nota, K3 MAIN ou canal fixo, K4 cor do pad; K5 ou OCT percorrem controles, PATCH volta à seleção pelo toque. PARA sai. BT salva os quatro mapas. Não há autosave. GLOBAL + REC solicita restaurar o mapa atual; BT confirma, PATCH cancela.

Pads: OFF, NOTE, CC momentâneo, CC toggle; AUTO e nove cores. Brilho respeita o ajuste global do SMKTRON. USB e TRS/DIN enviam as mesmas mensagens; sem Bluetooth, Mackie/HUI ou feedback de estado da DAW. Os botões disponíveis e as convenções são os mesmos da MM1a.

O simulador tem memória própria no navegador. Selecione uma saída Web MIDI para controlar um instrumento externo, ou use o monitor sem saída. MIDI IN musical é ignorado nos dois modos. Para instalar, feche o áudio e as portas MIDI do simulador/DAW; use Chrome/Edge no computador. O instalador privado exige o V016 original para retorno e confere modelo, tamanho, SHA-256 e CRCs. Há um instalador de retorno direto à GR3 oficial.

## Build e validação

| Item | GR3 | MM1 |
|---|---:|---:|
| App | 586.032 B | 598.364 B |
| Folga flash | 33.488 B | 21.156 B |
| RAM estática | 31.744 B | 33.360 B |
| Pool | 230.820 B | 233.204 B |

18 testes passaram: 11 de integração/áudio/dados, 3 regressões de pads e 4 de instalador/AudioWorklet/arquivos. Trechos de áudio comparados bit a bit; hash do header de samples idêntico ao GR3: `2f6d6c5d28383a61a888fb037d4b340cad34f8ebf47547f5849ec2696ba97b3d`. Protocolo, loader e primeiros 16 KiB do pacote idênticos ao GR3. Simulador verificado no navegador (início do áudio, controlador e editor). Os testes de boot antigos baseados em CH4 não se aplicam à nova luz de GRAIN do GR3; não foram contados como regressões aprovadas.

## Memória e compatibilidade

Cenas 0x9E000/0x9F000, acordes 0xA0000/0xA1000 e boot 0x9D000 permanecem no formato GR3. MIDI usa 0xA2000/0xA3000, duas cópias CRC com gravação do cabeçalho por último e leitura de conferência. Não usa áreas protegidas, OTA ou recursos de fábrica. Falha de gravação conserva a última cópia válida. O backup do instrumento não inclui os mapas MIDI. O retorno à GR3 deixa esses mapas guardados.

Samples e tabelas ADPCM inalterados. Não foi aplicada compactação dos checkpoints. Não foram removidos sons, GRAIN, sequenciador, camadas, cenas, acordes ou temas. A economia vem do reaproveitamento da interface e da integração com o código existente.

Build reproduzível: `engine/build.sh --board smk37 --model pro --release 0.3.0-mm1b`, com JieLi toolchain e SDK descritos em engine/docs; firmware incluído gerado com identidade 456 pelo tools/build.py correspondente. Consulte native-build.txt, test-results.txt e regression-results.txt. USB/TRS aprovados no teclado com MM1d; novas telas MM1e revisadas no simulador.

## English

Experimental SMK-37 Pro prototype, based on GR3 with the SMKSYNTH MM1a controller. Current identity SMK-37 Pro_460. USB/TRS physically approved in MM1d; MM1e enlarges the UI. GLOBE cycles SETUP → SYSTEM → PAD CHORDS → MIDI MODE; turn K1 right to enter. In controller mode, GLOBE opens GLOBAL / CONTROLLER and K1 left returns to the instrument. Boot always starts in synth mode.

Synth mode sends and receives no musical MIDI. Technical USB identity/update SysEx remains available. Controller mode silences internal audio and stops recording, sequencer and held chords. Incoming MIDI neither sounds nor passes through. Release keys, pads and pedal after switching. Playback never restarts automatically.

Four editable maps, USB + TRS/DIN, ABS accelerated 1/2/4/8 or REL 64 ± steps, A/B knob/fader/pad banks, velocity, octave, pitch bend, mod wheel, sustain and GLOBE + PLAY Panic. KNOB BANK and FADER BANK together switch pad banks. No Bluetooth, Mackie/HUI or DAW state feedback.

PARA opens the on-device editor. Touch a control to select it; K1 type, K2 CC/note number, K3 channel, K4 pad colour, K5/OCT control selection; PATCH picks by touch. BT explicitly saves all four maps. GLOBAL: K2 map, K3 main channel, K4 ABS/REL. REC asks to reset the current map; BT confirms and PATCH cancels. Connect the sustain pedal released before power-on for automatic polarity calibration.

MIDI maps have separate CRC-checked storage and are not included in the synth backup. GR3 boot, favourites, chords and scene formats are preserved. Samples and ADPCM tables are unchanged; no instruments or performance features were removed. The simulator uses independent browser storage; choose a Web MIDI output to control external equipment. Stop the simulator audio and close its MIDI ports before using the installer. Direct GR3 rollback is available. USB/TRS physically approved in MM1d; MM1e UI reviewed in the simulator.

## Credits

FMEL-1 / AntonioAltea, SMK-37 port / M. Geisler (genkigenki), Dinghy / zvenson, Felucca / Leo Kuroshita, SLOOP and OberVave contributions retain original notices. MIDI mapping/controller conventions derived from Snoop37 / M. Geisler and SMKSYNTH MM1a. SMKTRON direction, workflow and hardware approval: Marcus Padrini. Implementation and technical work: ChatGPT/Codex under his direction. GPL-3.0-only code; Taijiguy Mellotron samples retain their own noncommercial terms. See engine/LICENSING.md and source.zip.

## MM1b / TRS — 13 novembro 2026

USB aprovado no teclado pelo usuário; TRS sem resposta usando o mesmo cabo do SMKSYNTH. Reproduzido um bloqueio no roteamento nativo: a prontidão de TRS exigia espaço na fila USB. O Panic de entrada tem 80 mensagens, mais que a fila USB de 64 eventos; sem um consumidor USB, ele podia impedir notas posteriores em TRS. Isso é uma causa demonstrada em teste, mas ainda não confirma a causa exclusiva do relato físico.

MM1b retira essa dependência. A UART continua com sua fila e serviço no TIMER5. USB cheio passa por recuperação com Panic apenas no USB, após as mensagens antigas; notas USB novas ficam bloqueadas durante essa recuperação para evitar notas presas. Se USB não consome, retornar ao instrumento não fica esperando indefinidamente; mensagens antigas pendentes são descartadas antes de sair. A inicialização UART foi movida para depois do áudio/USB e antes do TIMER5, igual ao SMKSYNTH de referência. Mapas, formatos e regiões de salvamento não mudam.

`trs_ports_test.c` compila o cabeçalho real de roteamento nativo e a fila UART real com periféricos simulados: reproduz o bloqueio antigo, verifica TRS após 80 mensagens de entrada, USB conectado sem leitura, USB desconectado, recuperação USB sem resetar TRS, 400 mensagens com ambas as saídas e retorno ao synth sem MIDI musical nem notas antigas reaparecendo. Este teste cobre os periféricos de forma modelada; a emissão elétrica pelo jack ainda precisa da confirmação no teclado.

English: MM1b fixes a demonstrated native routing deadlock: an unread USB MIDI IN endpoint could fill its 64-event queue during the 80-event entry Panic and stop TRS output too. TRS no longer waits for USB capacity. USB overflow uses a USB-only safety reset; leaving controller mode discards stale unread packets after a bounded wait. UART initialization now follows audio/USB setup, before TIMER5, matching SMKSYNTH. Same maps and storage layout. Physical TRS confirmation is pending; automated peripheral tests cannot prove the electrical jack output.


## MM1c / diagnóstico físico — 13 novembro 2026

O usuário confirmou que MM1b continua sem notas e CCs no TRS, com cabo e destino já validados no SMKSYNTH. A alteração de MM1b permanece útil para independência USB/TRS, mas não resolveu o relato físico. A comparação com o source.zip do SMKSYNTH MM1a confirmou HAL UART e fila originalmente idênticos, mesmo pino PC7 e serviço TIMER5. Isso ainda não prova configuração elétrica equivalente no teclado.

MM1c (0.3.0-mm1c / SMK-37 Pro_457) é uma versão de diagnóstico, não uma correção confirmada. Toque algumas notas e mova um encoder no MIDI MODE, depois pressione GLOBE: o bloco inferior mostra Q (bytes aguardando envio), ACK (bytes cuja conclusão foi sinalizada pela UART), U (CON0 em hexadecimal), B (divisor baud), IO (IOMAP_CON3 em hexadecimal), CLK (fonte UART), PIN (cinco bits: OUT, DIR, DIE, PU, PD do PC7, nessa ordem a partir do bit menos significativo) e T (chamadas de serviço no TIMER5). O visor atualiza a cada 250 ms. ACK não comprova sinal no jack: comprova somente que o periférico sinalizou conclusão. Esperados: B 383, CLK 1, IO[23:20]=1 e bit7=0; PIN bits DIR=0, DIE=1, PU=1, PD=1. A foto permite distinguir fila bloqueada, UART sem conclusão e possível problema de roteamento físico. Não há envio automático de notas de teste.

Os contadores são voláteis e não alteram mapas ou formatos de armazenamento. Instrumento continua sem MIDI musical. Instalador MM1b anterior preservado em /install-midi-mm1b/; retorno GR3 aceita MM1c. O simulador sinaliza HARDWARE ONLY no bloco de diagnóstico, pois não modela os registradores físicos.

English: MM1b did not resolve the user's physical TRS failure. MM1c adds read-only UART/pin snapshots and timer/acknowledged-byte counters under controller GLOBAL (GLOBE). Play notes and turn a knob first, then photograph the screen. ACK means UART transmit completion, not confirmed electrical output at the jack. This is a diagnostic release; no physical fix is claimed. Samples, features and persisted maps remain unchanged.

## MM1d / leitura do teste físico e ajuste do HAL — 13 novembro 2026

Foto do usuário com MM1c: Q 0, ACK 1386, U 00008001, B 0, IO 00108808, CLK 1, PIN 1C, T 835586. A fila esvazia e UART sinaliza conclusão; isso não comprova bits corretos no jack. A leitura B=0 foi inicialmente suspeita, mas não permite essa conclusão: o header oficial WL82.h declara BAUD como somente de escrita. A leitura desse registrador foi removida.

Referência primária consultada: https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK/raw/release/AC79NN_SDK_V1.2.0/include_lib/driver/cpu/wl82/asm/WL82.h . UART: CON0/CON1/BAUD são uint16_t; BUF é uint8_t, em slots separados de 4 bytes. A biblioteca oficial cpu/wl82/liba/cpu.a confirma no IR de uart.c.o stores de i16 para CON0/BAUD e os seletores de gpio.c.o para o canal 1: PU=PD=1, OUT=0, DIR=0, DIE=1, IOMAP_CON3[23:20]=1 para UT1 TX. Esses bits de roteamento já coincidem com a foto. A diferença de largura de acesso é real; não está demonstrado que o acesso anterior de 32 bits seja a causa da falha física.

MM1d (0.3.0-mm1d, SMK-37 Pro_458): HAL passa a acessar controle/divisor com 16 bits, alinhado ao SDK. Ao entrar no controlador, reinicializa UART e roteamento PC7 dentro de seção crítica, antes de enfileirar o Panic. Mapas e dados do instrumento não mudam. Não há reinit durante um byte em andamento no uso normal: a saída do modo espera a fila/UART terminarem; o boot não enfileira MIDI musical.

Diagnóstico em GLOBE: Q/ACK permanecem; U passa a 4 hexadecimais e MIN é o menor tempo observado, em microssegundos, entre escrever um byte e observar a conclusão, medido com TIMER4 e consultado pelo TIMER5. Não é medição direta do baud: inclui a quantização/atraso do serviço de interrupção; não equivale a captura de bit. H e L contam amostras lógicas do registrador de entrada PC7 enquanto há byte em andamento. H/L são evidência de nível no pad do chip, não prova de corrente nem polaridade no conector externo. A leitura IN depende do comportamento do mux do chip. Contadores reiniciam ao entrar no modo. O bloco não promete medir velocidade elétrica. MM1c anterior preservada em /install-midi-mm1c/.

English: MM1c photo shows drained queue and UART completions, with expected routing bits. BAUD=0 is not a valid diagnosis because the vendor marks BAUD write-only. MM1d removes that read, uses vendor-matched 16-bit UART control/divider access, reinitializes the port at controller entry, and adds timer-based minimum completion latency plus logical PC7 high/low sample counts. These are diagnostic observations, not electrical verification of the jack. Physical TRS fix remains unconfirmed.


## MM1e / telas legíveis — 13 novembro 2026

Usuário confirmou que MM1d resolveu o TRS. USB e TRS aprovados no teclado. A causa física exata entre os ajustes de largura dos acessos e reinit na entrada não foi isolada; a versão funcional foi preservada. MM1e (0.3.0-mm1e / SMK-37 Pro_459) muda somente a apresentação do MIDI MODE e identificação da versão. Driver UART, fila, roteamento USB/TRS, entrada/saída de modo e mapas salvos permanecem os da MM1d.

GLOBAL e edição mostram quatro blocos: rótulo do knob e valor. Fonte de rótulo sobe de 8 para 13; valores usam 17. Zoom usa rótulos 17 e valores digitais 24. GLOBAL: K1 MODE / MIDI (esquerda volta ao synth), K2 MAP, K3 CH, K4 KNOBS / ABS ou REL. Edição: K1 TYPE, K2 CC/NOTE conforme o tipo, K3 CH, K4 COLOR (pads). Cabeçalho identifica o controle; K5 seleciona e PATCH permite escolher por toque. Campo sem uso mostra --. BT salva; PARA volta. Entrada destaca K1 RIGHT / ENTER MIDI MODE; confirmação de reset e seleção por toque também usam fontes maiores. Tela principal amplia mapa/canal, último controle/valor e indicação de bancos. Temas Light/Dark e Zoom reutilizam fontes existentes e o mesmo canvas dividido em dois tiles, sem nova alocação de framebuffer.

Diagnóstico TRS retirado da tela GLOBAL. A versão anterior continua disponível em /install-midi-mm1d/. Memória e sons preservados. Capturas renderizadas diretamente do WASM revisadas em Light, Dark e Zoom nas telas de entrada, GLOBAL, seleção e edição (incluindo valor127, canal16 e nome de cor longo).

English: User physically confirmed working USB and TRS in MM1d. MM1e only revises presentation and version identity: four large knob/value cards for GLOBAL and control editing, larger entry/pick/reset screens and main readouts. Labels use 13 and values17; Zoom uses17 labels and24 digital values. UART/USB routing, mode isolation, samples and persisted map format are unchanged. TRS diagnostics removed from everyday GLOBAL screen; MM1d installer retained. Existing fonts and tiled canvas reused.


## MM1f / K5 exclusivo do controlador — 14 novembro 2026

Usuário relatou K5 muito lento nos dois sentidos no MIDI MODE. Encontrado consumidor concorrente somente para esse encoder: platform_poll chama fm1_adc_read(FM1_ADC_MASTER), que no HAL do SMK37 implementa o volume como potenciômetro virtual e consome fm1_enc_take(4). Depois, mm_input tenta consumir os mesmos passos para o CC ou seleção de controle. A aceleração MIDI é igual para os oito encoders; a diferença está antes dela, na disputa pelos eventos.

MM1f (0.3.0-mm1f / SMK-37 Pro_460) condiciona a leitura do MASTER a !mm_on. Durante MIDI, K5 pertence integralmente ao controlador, incluindo ABS, REL e seleção na edição. Durante synth, a leitura e curva de volume são exatamente as anteriores. O master do synth não muda ao girar K5 no MIDI. Não muda decoder físico, aceleração, USB/TRS ou mapas salvos.

k5_test.py extrai e compila o platform_poll nativo novo, o platform_poll da MM1e preservada e o fm1_adc_read real do HAL; apenas serviços e a fila física de encoder são simulados. Reproduz a perda de K5 na MM1e, verifica passos positivos/negativos integrais com MIDI ativo, estabilidade de master/gain, serviços ativos no loop e o mesmo comportamento de volume do synth após voltar. O teste WASM sozinho não cobria essa disputa nativa, porque emula o painel. Captura física do encoder ainda depende do teste do usuário; uma eventual falha elétrica adicional não é descartada pelo teste de software. Versão anterior preservada em /install-midi-mm1e/.

English: MM1e native platform_poll was still consuming physical K5 events through the synth virtual MASTER-pot reader, before MIDI could use them. MM1f disables that reader while mm_on, preserving all encoder steps for MIDI ABS/REL and editor selection while retaining unchanged synth volume behavior. Native regression reproduces the old event loss and checks corrected ownership and volume preservation. Hardware confirmation pending.
