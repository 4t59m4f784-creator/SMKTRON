# SMKTRON — revisão de áudio da PM1 e protótipo AT1

27/10/2026. Revisão de código, testes determinísticos em WebAssembly e compilação nativa SMK-37 Pro. Não houve medição nem gravação no hardware nesta sessão. PM1 oficial preservada.

## Achados confirmados

1. **Mix de reverb sem rampa.** Vibrato, Leslie e echo já usavam rampas de20ms, mas o plate tinha ganho calculado por bloco com truncamentos inteiros sucessivos. Ligar/desligar e mover o fader alteravam o retorno de imediato. AT1 mantém os mesmos ganhos finais e usa rampa de20ms com resolução fracionária no retorno, sem multiplicações64bit novas por amostra.
2. **Histórico de vibrato congelado.** MIX0 suspendia o preenchimento do delay e a atualização do filtro; reativar podia ler conteúdo antigo. Além disso, quando a rampa terminava em0 no meio de um bloco, a fase podia avançar novamente por um bloco inteiro. AT1 mantém delay/filtro/fase em execução, inclusive em bypass global. A rampa de mix já existente foi preservada. O teste de reentrada converge bit a bit ao vibrato continuamente ativo; isso não prova eliminação de todo clique perceptível no teclado.
3. **Ganho final em degraus.** Master global é aplicado por bloco. K5 tem filtro no loop principal nativo, cuja cadência pode variar com a tela, mas não rampa por amostra no ganho final. AT1 suaviza por amostra o ganho efetivo, ligado em estéreo, sem alterar níveis finais ou curva nativa. O adaptador web recebe suavização equivalente em suas duas etapas de ganho.
4. **Escritas na flash interrompem áudio.** O driver desliga interrupções ao apagar setores e programar páginas. SAVE de boot e gravação de preset são explícitos e podem acontecer tocando; MIX/PAN dos acordes grava automaticamente após600ms, também sem esperar silêncio. O ensaio storage.json confirma chamadas de gravação com nota ainda sustentada. Faders normais de FX não provocaram gravação automática: não é correto atribuir o ruído deles à flash. AT1 NÃO modifica transações ou comportamento de salvamento. Uma correção específica exige fila em RAM, detecção de silêncio incluindo caudas e confirmação persistente após a escrita; quiet() atual só considera vozes, não caudas.
5. **Tela não equivale a flash.** LCD usa DMA e espera no loop principal; não achei desativação de interrupções no desenho normal. Isso reduz a suspeita de bloqueio direto, mas não demonstra ausência de disputa de recursos ou perda de prazo. DSP MAX/OVER e USB MAX/OVER do diagnóstico nativo precisam de leitura no teclado sob carga. WebAssembly não mede esses prazos.

## Artefatos sustentados em flauta/strings

Ensaio de16 combinações: quatro tapes × reverb0/19/50/80%, seis notas,16s cada. Nenhuma amostra na saída digital do simulador atingiu o limite de clipping. Há saturação intencional dentro do caminho de áudio e linhas int16 no plate; o ganho após essa saturação não desfaz harmônicos criados antes. Isso pode contribuir com sinais densos, mas não confirma a causa do relato.

Os loops usam crossfade350ms e pontos alinhados aos checkpoints do ADPCM; não encontrei erro óbvio de restauração do decoder. Isso não exclui textura/ruído das gravações, compressão ADPCM, loop audível ou sobrecarga real. Não se alteraram samples, loops, saturação, algoritmo/tamanho do reverb ou headroom sem prova.

**Outra diferença de validação:** o simulador herdado usa ganho de audição e volume K5 linear; o nativo usa K5 quadrático. As medições publicadas são comparações controladas do simulador, não medidas de nível absoluto/CPU do teclado. AT1 preserva essa relação de nível existente, suavizando as etapas. Uma equalização completa de curvas deverá ser uma mudança separada e explícita.

## Evidência e escopo AT1

Scripts: measure.mjs, transitions.mjs, storage.mjs e verify.mjs. Resultados: baseline.json, transitions.json, storage.json. O teste de transientes compara sinal com/sem evento de controle em instâncias iguais: redução do maior degrau do resíduo não é uma porcentagem de redução perceptiva do ruído.

AT1 trata as mudanças de mix/ganho e o histórico do vibrato. Não corrige flash em tempo real, roubo abrupto de voz ao esgotar polifonia, mudanças de timbre/layer durante sustain ou mudanças instantâneas de outros coeficientes (EQ, predelay, etc.). São pontos adicionais para testes dirigidos, não falhas audíveis confirmadas nesta sessão. A suavização de ganho se aplica antes do mute final e preserva silêncio ao atingir0; os formatos das memórias e a polifonia ficam iguais.

Nativo AT1:616832/619520 bytes,2688 livres; RAM29696/98304 (+16 sobre PM1); pool237860 inalterado. Guardas de build aprovadas, .ram_text971 instruções sem chamadas. O custo de manter vibrato aquecido exige validar margem de CPU no teclado. Instalador isolado; PM1 não foi substituída.

## Validação musical indicada

Comparar os mesmos acordes, tapes e parâmetros com PM1: alternar reverb e vibrato em notas sustentadas, mover faders lentamente, mover K5 e Master, testar flauta/violinos com mix alto, usar ambos os grupos de seis vozes, ARP/SEQ, navegar telas. Observar contadores de prazo no diagnóstico com KNOB BANK longo. Separadamente, verificar SAVE e MIX/PAN dos acordes para isolar a classe de interrupções de flash. Gravações USB ou de linha permitem localizar o instante dos estalos sem atribuí-los antecipadamente ao sample.


## AT2 — favoritos e transição, 27/10/2026

Favoritos mantém o dono dos pads (incluindo fixação e ciclo KNOB BANK), mas não desenha mais a página de slots. Ao entrar e após carregar, o display volta a PLAY ou LAYER conforme o favorito. O cabeçalho e a indicação do último preset nos LEDs continuam disponíveis.

Recall válido agenda uma redução do bus seco em512 amostras (~11,6ms), aplica os parâmetros no loop principal somente após chegar a0 e restaura o bus seco nas512 amostras seguintes. O estado dos buffers de delay/reverb não é reiniciado. As configurações de efeitos do favorito ainda são aplicadas e podem alterar a cauda audível, inclusive quando um favorito desliga um efeito. A troca não usa duas engines simultâneas. A duração total no hardware inclui a disponibilidade do loop principal; exige escuta física. Navegar para outro dono dos pads cancela recall pendente e restaura ganho, evitando um favorito aplicado tardiamente ou mute permanente. Slot vazio/inválido não inicia transição nem altera o som.

Testes verificam estado/página Patch/Layer, persistência do modo nos pads, aplicação após ganho0, recuperação a ganho unitário, manutenção de teclas físicas sustentadas, cancelamento e ausência de escrita na flash ao carregar. Comparação adicional de máximos de diferenças entre amostras (scene-measure.json) não apresentou melhora monotônica: essa métrica é dominada pelo próprio conteúdo harmônico e não deve ser apresentada como redução comprovada de cliques. A suavização é comprovada no caminho de controle e precisa ser avaliada musicalmente no teclado.

Não houve novo tratamento para o agudo metálico relatado; aguardamos gravação do usuário. Mantidas as melhorias AT1 e as pendências de flash/CPU descritas acima. Build AT2:616020/619520,3500 bytes livres, RAM29712, pool inalterado. Sem teste físico nesta sessão. PM1 oficial não foi alterada.
