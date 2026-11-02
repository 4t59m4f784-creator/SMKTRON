# SMKTRON GR3 — animação GRAIN e iluminação de performance

Protótipo privado derivado do GR2. Todos os controles, a migração de registros, o sequenciador e os salvamentos continuam com o comportamento do GR2. O registro permanece na versão 7, com 170 bytes.

## Animação de fragmentos

Em Light e Dark, fragmentos percorrem uma fita e atravessam uma cabeça de leitura. SIZE altera o comprimento dos fragmentos, SPREAD sua distribuição vertical e DENSITY sua quantidade. Há pequenos pontos desprendidos, inspirados na textura granular. É uma representação gráfica dos ajustes e da atividade das vozes, não uma medição da forma de onda nem um mostrador exato de cada grão de áudio.

Quando GRAIN está ligado, com MIX maior que zero, fora de bypass e há vozes soando, a animação avança. Ao terminar o decaimento das notas ela estaciona; desligar GRAIN também a estaciona e suaviza suas cores. Em Zoom, o movimento fica numa faixa de 12 pixels abaixo dos controles, mantendo os nomes e valores grandes.

A atualização animada isolada ocorre em intervalos de pelo menos 80 ms. Light/Dark redesenham somente o palco de 240×64; Zoom somente uma faixa de 240×12. Mudanças de controle/página continuam tendo atualização imediata. Não há sprites novos, consumo do gerador aleatório de áudio ou gravação de parâmetros pela animação.

## Pad 10 no modo Performance

| Estado | Luz |
|---|---|
| GRAIN desligado | Coral bem discreto |
| Ligado, sem notas soando | Coral constante |
| Ligado e processando notas | Pulsação suave entre coral e âmbar |
| Bypass de efeitos ou MIX zero | Coral atenuado, constante |

A pulsação usa SIZE e DENSITY para sua cadência visual, com ciclo limitado aproximadamente a 0,48–2 s. Usa o relógio de animação dos pads já existente, de 40 ms. O brilho global multiplica todas as cores e intensidades. Em Favoritos e Chord Pads, o pad mantém sua função e iluminação próprias; não recebe essa pulsação de GRAIN.

## Otimização aplicada

Removido o caminho de desenho legado da base FM-1 para o alvo SMK-37, após o despacho dos 14 renderizadores atuais. Isso permite ao linker descartar as funções e sprites que ficaram sem referência. As páginas atuais, boot e diagnóstico são preservados. A compactação de checkpoints não foi aplicada: os dados e o acesso aos samples permanecem como no GR2.

| Build nativo | App | Folga | RAM estática | Pool |
|---|---:|---:|---:|---:|
| GR2 | 619.520 B | 0 B | 31.776 B | 237.860 B |
| GR2 apenas com a limpeza | 584.716 B | 34.804 B | 31.728 B | 230.820 B |
| GR3 com animações | 586.032 B | **33.488 B** | 31.744 B | 230.820 B |

A nova apresentação consome 1.316 bytes sobre a versão limpa. Limites permanecem app 619.520 B, RAM estática 98.304 B e pool 344.064 B. Nenhum banco de salvamento foi movido.

## Simulador e compatibilidade

GRAIN continua no botão físico ARP, K6 seleciona os estilos na sua página e o pad 10 liga/desliga em Performance. A roda controla vibrato e chama sua página, com K4 definindo MAX MIX. Favoritos e Boot SAVE guardam todos os ajustes de GRAIN, seu estilo e ativação.

O simulador GR3 usa chaves próprias. Na primeira abertura nesse mesmo navegador e origem, copia os dados GR2 existentes para suas chaves, sem modificar GR2. Depois os dois simuladores são independentes. A versão oficial AT2 e os protótipos anteriores permanecem preservados.

## Verificação

Passaram 14 testes específicos de GR3/GR2 e 20 de regressão de acordes/pads, além do AudioWorklet real em Node. Foram verificados movimento e pausa nos três temas, estabilidade dos valores em Zoom, cores/pulsação, escala de brilho, prioridade dos modos de pads, equivalência do áudio com GR2 e igualdade dos bytes salvos. O build nativo passou os limites de memória e a checagem de código em RAM (971 instruções, sem chamadas).

Os arquivos `animation.gif`, `preview.png` e as capturas de tela vêm dos framebuffers e valores RGB de LEDs produzidos pelo firmware WASM. A prévia HTTP é usada para QA do painel web; o processamento do AudioWorklet é verificado separadamente em Node. Instalador GR3 disponível em ../install-grain3/. Build nativo validado; teste físico pendente. O usuário aprovou o simulador.

Fonte e licenças: `source.zip`. Código adicional em `experiments/grain3/`; medições da limpeza em `experiments/space/gr2-audit/` no repositório.

## Instalação GR3

Use Chrome/Edge no computador e USB de dados. Separe o V016 original para a conferência do retorno. Feche DAWs e o simulador antes de conectar. Confira 0.3.0-gr3 no boot. Teste GRAIN (ARP), K6, pad 10, roda, SEQ, favoritos e Boot SAVE nos três temas. Os registros v6 existentes são migrados ao carregar; novos salvamentos usam v7. A AT2 não entende v7; antes de salvar nesta versão, anote os ajustes que deseja manter ao retornar. Não inclui novos samples.

## Instalação GR3

Use Chrome/Edge no computador e USB de dados. Separe o V016 original para a conferência do retorno. Feche DAWs e o simulador antes de conectar. Confira 0.3.0-gr3 no boot. Teste GRAIN (ARP), K6, pad 10, roda, SEQ, favoritos e Boot SAVE nos três temas. Os registros v6 existentes são migrados ao carregar; novos salvamentos usam v7. A AT2 não entende v7; antes de salvar nesta versão, anote os ajustes que deseja manter ao retornar. Não inclui novos samples.
