# Prévia de backup do SMKTRON

Para o SMK-37 Pro com SMKTRON ML1 (USB `SMK-37 Pro_463`). O firmware de teste BK1 usa a mesma área de dados da ML1 e acrescenta o protocolo de backup USB. O firmware oficial ML1 ainda não atende aos comandos da ferramenta.

Na raiz do repositório, execute `python3 -m http.server 8000 --bind 127.0.0.1 --directory web`. Abra [http://localhost:8000/backup/](http://localhost:8000/backup/) no Chrome ou Edge. Na seção de firmware, atualize da ML1 para a prévia. Depois do reinício, clique em **Conectar teclado**.

Salve suas alterações no instrumento antes de exportar. O arquivo contém as oito posições de cenas, as oito posições de acordes e seu mix, o Boot Save com a sequência e os ajustes globais, e os quatro perfis MIDI salvos. O arquivo é processado no computador e no instrumento.

Para restaurar, baixe primeiro uma cópia atual do teclado, selecione o arquivo desejado e confira os pads preenchidos. Pare PLAY e REC, saia do MODO MIDI e solte teclas, pads e pedal antes de confirmar. A restauração substitui o banco salvo inteiro. Desligue e ligue após a verificação. Se a gravação falhar com o teclado ainda ligado, reconecte e repita com o mesmo arquivo. Uma perda de energia pode deixar algumas partes do banco já restauradas. Após religar, exporte o estado que o teclado carregou e tente a restauração novamente.

Para voltar ao firmware ML1, use **Voltar à ML1** na mesma página. Os pacotes da prévia e da ML1 têm seus tamanhos, identidades e SHA-256 conferidos antes da instalação.

O teste de ponta a ponta simula a memória do instrumento e roda com `node --test tests/backup.test.mjs` (Node 22 e compilador C). Para recompilar o pacote, use `sh tools/build_backup_preview.sh` com o SDK e o toolchain do JieLi configurados.
