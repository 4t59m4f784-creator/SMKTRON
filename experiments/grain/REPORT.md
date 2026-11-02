# SMKTRON GR1 — fragmentos de fita

Protótipo privado para avaliar o resultado musical. A versão oficial AT2, o teste Zoom ZM1 e o diagnóstico USB UD1 continuam preservados.

## Como experimentar

1. Clique em **Iniciar áudio**. Toque pelo teclado da tela, pelas letras do computador ou pela entrada MIDI do navegador.
2. Abra **GRAIN**. No teclado físico, essa função ocuparia o botão **ARP**. Neste protótipo, o botão abre a página; quem define a quantidade do efeito é o MIX.
3. Use os quatro controles ou os exemplos **Bruma**, **Pulsos** e **Fragmentos**. **Fita original** retorna MIX a 0%.
4. Escolha FLUTE, CHOIR, CELLO ou VIOLINS pelos pads de performance ou K8. Também funciona com Layer e Pad Chords.

| Controle | Faixa | Função |
|---|---|---|
| SIZE / K1 | 20–400 ms | Duração de cada fragmento |
| SPREAD / K2 | 0–100% | Variação aleatória da posição de leitura, até aproximadamente ±0,75 s |
| DENSITY / K3 | 15–100% | Intervalo entre fragmentos: mais baixo cria pausas; mais alto sobrepõe |
| MIX / K4 | 0–100% | Fita normal / granulação; transição suavizada |

Começa em **120 ms / 35% / 85% / MIX 0%**. A granulação afeta todas as vozes ativas, incluindo Layer e acordes, antes dos efeitos compartilhados. Em Chord Mode, GRAIN pode ser aberto sem desligar os pads; PATCH retorna à tela de acordes.

Os quatro ajustes GRAIN são **temporários nesta avaliação**: não entram nos presets nem no Boot SAVE. Os demais salvamentos continuam com seu formato anterior. Este simulador utiliza chaves próprias no navegador, sem importar ou alterar os dados dos outros simuladores.

## Implementação

Leitura granular dos samples ADPCM já existentes, com até dois grãos simultâneos por voz. Cada grão tem cabeças próprias para o patch principal e eventual layer. Janela suave com início/fim em zero, mistura com rampa curta e margem de nível no caminho granular. Não há captura de áudio, novo buffer longo, alocação dinâmica ou escrita de memória persistente no processamento granular.

SPREAD usa os checkpoints de ADPCM existentes: resolução de posição de 256 samples / 11.025 kHz, aproximadamente 23,2 ms. SIZE continua medido no áudio de saída. Portanto, este primeiro modelo privilegia custo previsível; ainda não inclui freeze, reverse ou dispersão de afinação. Não há compensação automática de loudness: densidades menores ficam mais espaçadas e podem soar mais baixas.

RNG próprio para os grãos. MIX 0% evita a decodificação granular e mantém o áudio original bit a bit nos cenários comparados.

## Arpejador e espaço

Somente neste protótipo, a geração de notas do arpejador e sua visualização foram retiradas. Seu botão/página passam a GRAIN. O sequenciador mantém gravação, reprodução, gate, swing e relógio interno. A estrutura dos registros persistidos e a numeração das páginas continuam iguais, incluindo campos antigos reservados, para evitar uma migração prematura.

| Build nativo | Código do app | Livre | RAM .data + .bss | Pool |
|---|---:|---:|---:|---:|
| ZM1 | 618.920 B | 600 B | 29.712 B | 237.860 B |
| ZM1 sem arpejador | 616.980 B | 2.540 B | 29.008 B | 237.860 B |
| GR1 | 618.892 B | 628 B | 31.744 B | 237.860 B |

Limites: app 619.520 B; RAM 98.304 B; pool 344.064 B. A retirada do arpejador recuperou **1.940 bytes** de código e **704 bytes** de RAM. A primeira granulação mais interface consome aproximadamente esse ganho. A margem de código continua curta.

## Validação e limites

Compilação WASM e compilação nativa SMK-37 Pro verificadas. Passaram 7 testes específicos de GR1 e 20 verificações de acordes/pads. O AudioWorklet real também foi exercitado em Node. As comparações antigas contra AP8/CH2 não foram reutilizadas como critérios de áudio: a referência correta é o ZM1/AT2, comparada diretamente pelos testes GR1. Testes cobrem bypass idêntico ao ZM1 para quatro tapes com/sem layer, limites dos controles, geração de grãos, extremos com 12 notas, liberação, sequenciador comparado ao ZM1, acordes e persistência, compatibilidade de tamanho dos registros e renderização Light/Dark/Zoom. O teste de silêncio avalia o componente variável do sinal, pois os efeitos originais já apresentam um pequeno piso DC em ponto fixo.

O áudio é processado no código C do instrumento; a página não aplica um efeito JavaScript externo. Testes em computador não comprovam prazo de processamento, consumo ou ausência de glitches no hardware. Com MIX ativo há cabeças adicionais e maior custo de processamento. **Ainda não há teste físico nem instalador GR1 publicado.** A primeira decisão é musical: quais resultados merecem avançar para otimização e teste no teclado.

Fontes e licenças estão em `source.zip`. Arquivos de validação: `experiments/grain/` no pacote de fontes.
