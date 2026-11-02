# SMKTRON AP8 — Padrão inicial oficial

Base funcional: AP7 testada no teclado por Marcus. AP8 altera apenas os valores iniciais. Primeiro boot sem SAVE: FLUTE, VOL 60% (614/1023), MASTER 70%, Fader Bank B, Light, pads 75%. Valores das 12 fotos e mensagem de 25/10/2026; sequenciador mantém o padrão vazio e parado. SAVE válido existente tem prioridade, incluindo Master e aparência. Cenas existentes não são alteradas.

- K5: volume da cena, fixo no topo de PLAY e LAYER, entre SMKTRON e TAPE/LAYER (VOL 0–100%). Atualiza ao mover K5 ou carregar cenas e permanece visível nos dois temas, inclusive junto de mensagens, modo de cenas e bateria baixa. Nas demais páginas, indicação SCENE VOL durante 1,4 s ao mover K5.
- Bateria sempre visível no cabeçalho das 13 páginas: quatro faixas pelos limites ADC 530/560/602 documentados para o SMK-37 original, sem percentual de carga inventado. Uma barra vermelha indica a faixa baixa; leitura ausente/inválida mostra contorno vazio cinza. Filtro de 1 s e três leituras consecutivas antes de mudar de faixa. Não infere carregamento pela conexão USB. Falta confirmação física dos níveis na unidade do usuário. Referência: genkigenki/fm1-to-smk37 docs/01-hardware.md §6.6, commit 54252ffa2219e3c9cd7eb70f4e99b361eaa048e1.
- GLOBE: GLOBAL 1/2 (SETUP) → GLOBAL 2/2 (SYSTEM) → GLOBAL 1/2.
- SYSTEM K1: PAD BRIGHTNESS 100%, 75%, 50%, 25%; K2: THEME LIGHT / DARK; K3: MASTER 0–100%, passos de 1%; K4 sem função.
- MASTER é uma atenuação global da saída. 100% mantém exatamente a saída anterior; 0% silencia. Não há ganho adicional. Aplica antes do limitador final e da captura USB no hardware.
- SAVE breve grava o estado de boot, inclusive Master, tema e brilho. Salvar/carregar cenas não modifica esses três ajustes globais nem os grava no boot. O volume K5 continua musical e é restaurado pelas cenas.
- Boot e cenas continuam com registros de 170 bytes, mesmos setores, CRCs e protocolo. No boot, o byte HALF usa bit 0 para HALF e bits 1–7 para (100 - MASTER). Essa codificação só existe durante a gravação/leitura do boot: o estado em memória e todas as cenas continuam com HALF 0/1. Cenas antigas permanecem legíveis e cenas novas são compatíveis com a AP3. SAVEs antigos iniciam em MASTER 100%. Um retorno à AP3 ignora a atenuação e pode restaurar HALF desligado se o boot contiver a codificação AP8; os parâmetros musicais e as cenas permanecem.
- Notas ativas laranja no Dark, brilho proporcional e correção de ataque/volume da flauta preservados. Nenhum sample novo.

Compilar: Zig 0.13.0, python3 tools/build_appearance.py; hardware: JieLi 20260730.1 e SDK documentado no engine, python3 tools/build_appearance.py --native. Definir ZIG, JIELI_TOOLCHAIN e AC79_SDK. Testes: node --test tests/appearance.test.mjs tests/lab.test.mjs (comparações históricas usam WASMs de referência preservados no repositório do laboratório).

Firmware 7dd9edb2bb90809b8e952b463a587dbf2c26789879a74a448ad4362d0d1ab63f; aplicação 617224/619520, livre 2296 bytes. Bootloader e primeiros 16 KiB idênticos à FL1 preservada. Base AP7 testada no teclado; novos valores iniciais autorizados pelo usuário e validados em software.

Licenças e créditos em engine/LICENSING.md e CREDITOS.md. Gravações Taijiguy mantêm termos próprios, separados da GPL.
