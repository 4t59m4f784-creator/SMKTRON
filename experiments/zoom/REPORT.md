# SMKTRON ZM1 — Zoom em teste

Base AT2 oficial, com um terceiro tema global: Light / Dark / Zoom. O estado de boot válido tem prioridade e o primeiro boot continua Light, VOL 90%, Master 70%, pads 75%, Fader Bank B e fita ativa.

## Como testar

1. No simulador ou teclado, pressione GLOBE duas vezes até GLOBAL 2/3, SYSTEM.
2. Gire K2 no sentido horário até THEME = ZOOM. K1 continua brilho dos pads; K3 continua Master.
3. Confira as 14 páginas, os quatro timbres, uma combinação de Layer, ARP/SEQ e Chord Pads. Os quadros seguem K1/K2 na linha superior e K3/K4 na inferior. A borda identifica o controle acionado.
4. SAVE breve guarda o tema para o próximo boot. O tema não acompanha as cenas; permanece global. No simulador, SAVE guarda os dados neste navegador, separadamente do teclado.
5. Para voltar à aparência anterior, SYSTEM → K2 → Dark ou Light. O diagnóstico técnico e a abertura mantêm seu desenho.

## O que mudou

- Layout compartilhado de quatro quadros, nomes ampliados e valores legíveis. Cabeçalho fixo por página, sem repetir o parâmetro acionado.
- Patch conserva as quatro cores da paleta Dark original no nome e box do timbre.
- Layer ativo mostra os dois timbres no título, cada um com sua cor. Com Layer OFF, mostra o patch e informa LAYER OFF.
- Sequenciador mantém passos e transporte; ARP mantém estado e BPM; Chord mantém MODE, MIX, PANs e instrução de gravação/HOLD. Notas tocadas ficam laranja no teclado compacto.
- Valores e unidades vêm da formatação original da engine. As fontes são as já existentes, portanto o aspecto real não é pixel a pixel igual à tipografia do esboço.
- Light/Dark e os controles musicais continuam disponíveis. Nenhum algoritmo de áudio, sample, efeito, voz ou polifonia foi alterado.

## Dados e compatibilidade

O Zoom usa o bit 6 antes não utilizado do campo de preferências do SAVE (sem aumentar seu tamanho). O acesso à paleta continua sendo booleano para evitar índices fora do intervalo. Light e Dark antigos são interpretados como antes; as cenas excluem as preferências globais. A instalação preserva os setores de boot, cenas e acordes.

Ao voltar à AT2, um SAVE com Zoom volta visualmente a Light porque a versão antiga ignora o novo bit; os parâmetros musicais permanecem compatíveis. A preferência pode ser escolhida novamente pelo usuário.

O simulador ZM1 usa chaves próprias e, uma única vez, copia os dados anteriores disponíveis no mesmo endereço. Ele não modifica os dados dos outros protótipos. Dados de outro endereço e dados do teclado não são transferidos automaticamente.

## Espaço e validação

App nativo: 618.920 / 619.520 bytes, **600 bytes livres**. RAM .data+.bss: 29.712 / 98.304; pool: 237.860 / 344.064, inalterados em relação à AT2. O empacotamento de glifos Terminus economiza 967 bytes de dados, verificando cada caractere byte a byte. Não reduz pixels nem remove fontes. O código RAM de flash passa a verificação de 971 instruções, sem chamadas.

30 testes automatizados: 25 de regressão musical/navegação/boot e 5 específicos do Zoom. Incluem persistência do tema, troca de cenas, Light/Dark pixel a pixel contra a AT2 nas 14 páginas, PCM idêntico no trecho comparado contra a AT2, renderização das 14 páginas e notas laranja que apagam ao liberar a tecla. São testes de software; não comprovam tempo de CPU, transferência de tela e estabilidade no hardware real. O novo desenho aguarda teste físico de leitura e execução.

## Distribuição

ZM1 é protótipo privado; AT2 continua oficial. A UD1 para o teste USB fica preservada em seu instalador separado. Não há alterações de USB nesta versão Zoom. Instale pelo Chrome/Edge de um computador e confira 0.3.0-zm1. O V016 original continua obrigatório como arquivo de retorno validado. Nenhuma gravação começa ao abrir ou conectar ao instalador.

Retorno à AT2: https://smktron-rf1.padrini.chatgpt.site/install/
