# SMKTRON GR2 — GRAIN persistente e vibrato pela roda

Protótipo privado. AT2 oficial, ZM1, GR1 e o diagnóstico USB UD1 permanecem nos seus endereços anteriores.

## Controles

| Controle | Função |
|---|---|
| GRAIN (botão físico ARP) | Abre a página GRAIN, sem ligar/desligar |
| K1 / K2 / K3 / K4 na página GRAIN | SIZE 20–400 ms / SPREAD 0–100% / DENSITY 15–100% / MIX 0–100% |
| K6 na página GRAIN | FREE / BRUMA / PULSOS / FRAGMENTOS, com indicação visível |
| Pad 10 em Performance (antigo Vibrato) | Liga/desliga GRAIN, preservando MIX, estilo e demais valores |
| Roda de modulação / MIDI CC1 | Controla o MIX efetivo do vibrato e abre sua página ao movimentar |
| K1 / K2 / K3 / K4 em VIBRATO | RATE / DEPTH / TONE / MAX MIX |
| SEQ, SEQ PLAY e SEQ REC | Sequenciador mantido |

K6 continua navegando entre páginas nas outras seções. Na página GRAIN, passa a selecionar o estilo. Alterar qualquer parâmetro pelo K1–K4 muda o estilo para FREE, mantendo o efeito ligado ou desligado como estava.

Os exemplos do topo são atalhos de audição: escolhem o estilo e ligam GRAIN. **Fita original** desliga GRAIN sem zerar o MIX. Esses atalhos não salvam automaticamente.

O antigo pad de vibrato só muda de função no banco Performance. Em Favoritos e Chord Pads, continuam valendo as funções específicas desses modos. O LED do pad 10 acompanha GRAIN, não a roda.

## Vibrato e foco da tela

MIX efetivo = MAX MIX × posição normalizada da roda. MAX MIX em 60% com roda na metade produz aproximadamente 30%. A roda na base produz zero; a zona morta física já existente é preservada. O DSP não adiciona mais profundidade automaticamente ao aumentar a roda: DEPTH fica sob controle do K2.

Movimentar a roda acima de zero abre VIBRATO. Voltar a zero retorna à página anterior, inclusive GRAIN ou Chord Pads. Navegar manualmente cancela esse retorno automático; uma nova movimentação acima de zero pode chamar VIBRATO novamente. A troca de tela não muda os modos dos pads e não interrompe acordes ou sequenciador. MIDI CC1 segue a mesma interação.

RATE, DEPTH, TONE e MAX MIX são salvos; a posição instantânea da roda não é salva.

## Salvamento e compatibilidade

Boot SAVE e cada favorito guardam SIZE, SPREAD, DENSITY, MIX, estilo e estado ON/OFF do GRAIN. Desligar mantém todos os valores. O bypass geral de efeitos também retira GRAIN suavemente sem apagar seu estado configurado.

O registro continua com **170 bytes**. Os quatro campos aposentados do ARP foram reaproveitados, com versão de registro **7**. Ao carregar boot ou favorito da versão 6, a migração preserva os demais parâmetros e a sequência, coloca GRAIN em FREE / 120 ms / 35% / 85% / MIX 75% / OFF e, se o antigo MIX do vibrato era zero, define MAX MIX como 100% para permitir seu uso pela roda. Um máximo anterior diferente de zero é preservado. Boot v5 utiliza a migração já existente antes dessa adaptação.

A migração acontece em RAM: favoritos antigos não são regravados automaticamente. Novos salvamentos usam v7. Firmwares antigos não interpretam os novos registros v7 como configurações válidas; não se promete compatibilidade reversa. Por isso, este simulador usa armazenamento próprio no navegador, separado dos demais protótipos e do oficial.

Master, tema e brilho dos pads continuam globais. Os acordes continuam em seu banco global separado. Os ajustes e a ativação do GRAIN são parâmetros musicais de cena.

## Interface

Light e Dark mantêm cabeçalho, ilustração de fragmentos de fita, teclado e quatro parâmetros inferiores. Zoom usa quatro blocos grandes. Todas as três páginas mostram K6 STYLE e a seleção atual, além do estado do pad/efeito. VIBRATO mostra MAX MIX no K4; no Zoom há também indicação do MIX efetivo da roda.

## Validação

Testes específicos verificam K6, FREE, pad 10, boot, favoritos, migração de registros antigos, roda física, MIDI CC1, foco/retorno, navegação manual, preservação dos modos de pads, bypass, PCM com GRAIN desligado, sequência e renderização dos três temas. A suíte de acordes/pads também foi executada, com comparações de áudio atual feitas contra ZM1, em vez das antigas AP8/CH2. O AudioWorklet real é exercitado em Node; a prévia HTTP é utilizada apenas para conferir a interface do navegador.

A compilação nativa ocupa **619.520 de 619.520 bytes do app: zero bytes livres**. RAM .data + .bss: 31.776 de 98.304 bytes; pool: 237.860 de 344.064 bytes. O código cabe exatamente, mas precisa recuperar margem antes de disponibilizar o teste no teclado (ver `native-build.txt` nas fontes). Não há teste físico nem instalador GR2 publicado. Uso de CPU e áudio em condições extremas no teclado continuam pendentes. A granulação usa o mesmo algoritmo musical do GR1, com rampa de mistura ao ligar/desligar.

Código e licenças: `source.zip`. Fontes adicionais em `experiments/grain2/`.
