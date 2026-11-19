# SMKTRON 0.3.0-elite1 — teste SMK-37 Elite

Experimental: instalação e retorno ainda não testados em Elite físico. Base ML1, incluindo MIDI Layers, melhoria de latência e pitch bend de 14 bits. Instalador e firmware oficiais do Pro permanecem intactos. Esta página não é anunciada na home e solicita aos buscadores que não a indexem.

1. Comece no **SMK-37 Elite V016 oficial**. Baixe o pacote do seu modelo em https://www.m-vave.com/product?id=smk-37-elite . O arquivo esperado é `SMK-37 Elite_016.fwsc`, 705236 bytes, SHA-256 `b2ee38abfef3ae922e4eecdb8a551e14345300ae6f270265feacd33b6ef99cb6`. Não é redistribuído aqui.
2. Faça backup dos dados atuais. Outro firmware identifica seu Elite como Pro? Retorne ao oficial Elite usando o instalador daquele projeto primeiro. Não force detecção cruzada.
3. Abra esta página em Chrome/Edge no computador, feche DAWs e outros programas MIDI. Carregue o V016 original, conecte e consulte; só então confirme a instalação. Nenhuma escrita ocorre ao abrir a página ou consultar o modelo.
4. Espere a identidade `SMK-37 Elite_463` e a tela `0.3.0-elite1`. Teclas/pads/encoders/faders/rodas usam o mesmo mapeamento de hardware da base SMK-37.
5. Teste 37 teclas (incluindo extremidades e acordes), áudio/fones, quatro timbres, FX, layer, pads, sequenciador, salvamento e reinicialização. Teste MIDI USB/TRS, canais solo e layers no banco B, bend lento, modulação, pedal e Panic (GLOBE + SEQ PLAY).
6. Confirme também o retorno ao oficial **Elite V016**, com o arquivo original selecionado. Guarde o log e confira teclas, rodas e áudio no firmware oficial.
7. Interrupção: reabra esta mesma página e navegador, selecione novamente o V016 e consulte. Retomadas exigem registro local da origem, operação e hash; a origem é preservada mesmo após várias interrupções. Não alterne instaladores no meio. Se não iniciar, a base oferece recuperação USB ao ligar segurando OCT−; sua identidade Elite_000 permite somente retorno ao V016. Recuperação física ainda precisa ser validada.

## Verificação técnica

Compilação própria com `MODEL=elite`: identidade do app Elite_463, loader Elite_900 e registro de retorno Elite_016. A proteção por modelo permanece estrita; nenhum `anyModel` é habilitado. Pacotes incorretos, Pro, FM-1 e MKE-P37 são recusados. O arquivo oficial de retorno e o pacote experimental são conferidos por identidade, tamanho, SHA-256 e CRCs antes da operação.

App: 603068 / 619520 bytes; RAM estática: 33712 bytes. Fontes: [firmware](../../firmware), [build Elite](../../experiments/elite/build.py). Consulte manifest.json para os hashes. GPL-3.0-only e avisos originais preservados; samples Taijiguy com termos de uso não comercial, conforme créditos principais.

Referências: https://github.com/genkigenki/fm1-to-smk37 e https://genkigenki.github.io/smk37/help.html . Testes em software não substituem a validação no Elite físico.

## English

Experimental Elite-only installer based on ML1; physical installation, recovery and stock return are **not yet validated**. Official Pro installer and firmware are unchanged. Start on official **SMK-37 Elite V016**, back up your data, and keep the original Elite stock package above. Do not use Pro V016 or force a keyboard reporting another model. Use desktop Chrome/Edge, load the original return package, identify the keyboard and confirm the operation. Expect Elite_463 and version 0.3.0-elite1. Test audio, all physical controls, saved settings, MIDI USB/TRS channel layers, 14-bit bend, sustain and Panic, then verify return to Elite V016 and save the log. Resume only from this page and browser with the same selected stock file. OCT− boot recovery is inherited from the base but untested on physical Elite; Elite_000 only enables stock return. Never use a dump from another unit.
