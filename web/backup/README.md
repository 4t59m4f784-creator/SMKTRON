# SMKTRON Backup · BK2

O instalador USB-MIDI atualiza o SMK-37 Pro diretamente da ML1 (`SMK-37 Pro_463`) ou da prévia BK1 (`_464`) para BK2 (`_465`). O botão **Voltar à ML1** instala o pacote oficial. Ambos conferem o tamanho, a identidade, o SHA-256 e os CRCs do pacote e verificam a versão após o reinício. A BK2 usa a mesma área de dados da ML1 e acrescenta a comunicação de backup e o registro de recuperação da restauração.

Para executar localmente, abra um terminal na raiz do repositório e rode `python3 -m http.server 8000 --bind 127.0.0.1 --directory web`. Abra [localhost:8000/backup/](http://localhost:8000/backup/) no Chrome ou Edge no computador.

1. Atualize para BK2. Depois do reinício, clique em **Conectar teclado**.
2. Salve no teclado o que deseja guardar e clique em **Baixar backup**. O arquivo `.smktron` inclui as oito posições de favoritos (cenas), as oito posições de acordes e seu mix, o Boot Save com a sequência e os ajustes globais, e os quatro perfis MIDI salvos. O arquivo é processado no navegador e no instrumento.
3. Para restaurar, exporte uma cópia atual, selecione o arquivo e confira as posições preenchidas. Pare PLAY e REC, saia do MODO MIDI, solte as teclas, os pads e o pedal, e feche os aplicativos que estiverem usando o áudio USB do teclado. A restauração substitui o banco salvo inteiro. Após a confirmação, desligue e ligue para carregar o banco restaurado.

A BK2 confere o arquivo completo antes de gravar os bancos. Se a energia cair depois de o teclado registrar a restauração na flash, ele retoma a operação na próxima inicialização. Se a interrupção ocorrer antes desse registro, a gravação dos bancos não chega a começar. Ao reconectar, exporte e confira o estado atual.

A BK2 é uma versão de teste para o Pro. A compilação e o fluxo USB foram verificados com um dispositivo simulado. O primeiro teste no teclado físico está pendente.

`node --test tests/backup.test.mjs` executa o teste de ponta a ponta (Node e compilador C). O pacote pode ser reconstruído com `sh tools/build_backup_preview.sh`, com o SDK AC79 e o toolchain JieLi configurados.
