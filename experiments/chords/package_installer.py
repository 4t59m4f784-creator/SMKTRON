#!/usr/bin/env python3
"""Separate CH3 hardware-test installer. Preserve AP8 production and verified USB transport."""
from pathlib import Path
import shutil,hashlib,json,re
R=Path(__file__).resolve().parents[2];T=R/'templates/installer';D=R/'dist/install';D.mkdir(exist_ok=True)
FW=R/'experiments/chords/build/engine/build/mel-0.3.0-ch4-smk37.fwsc';raw=FW.read_bytes();sha=hashlib.sha256(raw).hexdigest();name='smktron-0.3.0-ch4-pro.fwsc'
(D/'firmware').mkdir(exist_ok=True);shutil.copy2(FW,D/'firmware'/name)
for f in ['protocol.js','package.js','installer.css','LICENSE.txt','factory-preset.json']:shutil.copy2(T/f,D/f)
preset=json.loads((D/'factory-preset.json').read_text());preset.update(version='0.3.0-ch4',scene_volume_percent=90,scene_volume_raw=921);(D/'factory-preset.json').write_text(json.dumps(preset,indent=2)+'\n')
s=(T/'service.js').read_text().replace('SMKTRON 0.3.0-ap8','SMKTRON 0.3.0-ch4').replace('7dd9edb2bb90809b8e952b463a587dbf2c26789879a74a448ad4362d0d1ab63f',sha).replace('smktron-0.3.0-ap8-pro.fwsc',name).replace('smktron.installer.appearance.ap8.pending.v1','smktron.installer.chords.ch4.pending.v1')
(D/'service.js').write_text(s)
s=(T/'controller.js').read_text().replace('0.3.0-ap8','0.3.0-ch4').replace('smktron-installer-log.txt','smktron-ch3-installer-log.txt');(D/'controller.js').write_text(s)
s=(T/'index.html').read_text().replace('SMKTRON — Cenas 0.3.0-ap8','SMKTRON — CH4 · Oficial').replace('AP8 · ESTÁVEL E OFICIAL','CH4 · ESTÁVEL E OFICIAL').replace('Simulador AP8','Simulador CH4').replace('href="../"','href="../chords-preview/"').replace('href="../credits.css"','href="../chords-preview/credits.css"')
s=s.replace('Um caminho de ida.<br>O arquivo de volta.','Pad Chords.<br>Seu SMKTRON completo.')
a=s.index('<div class="notice">');b=s.index('</div>',a)+6
s=s[:a]+'''<div class="notice"><strong>SMKTRON CH4 — versão oficial.</strong> O firmware cabe com 4.248 bytes livres. Inclui oito acordes globais, HOLD, mix e pan por voz, nova paleta rosa/lilás e indicação pulsante do último preset carregado. ARP e SEQ preservados. GLOBE até GLOBAL 3/3 → PAD CHORDS; K4 MODE ON. K1 MIX, K2 PAN MELODY, K3 PAN CHORDS. Pads: 1–8 acordes; 9 REC; 10 HOLD; 11–14 timbre; 15 STOP/cancelar; 16 sair. Para gravar: REC → destino → toque o acorde → solte todas as teclas. Seis vozes para teclado e seis para acordes. Primeiro boot: volume da cena 90%, Master 70%, Light e pads 75%. O SAVE existente tem prioridade; a instalação não inclui presets de acordes. Acordes do simulador não são transferidos ao instrumento. A AP8 oficial permanece disponível para retorno: <a href="https://smktron-rf1.padrini.chatgpt.site/previous-ap8/install/">abrir instalador AP8</a>.</div>'''+s[b:]
s=s.replace('SYSTEM · 0.3.0-ap8 · oficial','PAD CHORDS · 0.3.0-ch4 · oficial')
s=s.replace('Oito snapshots musicais. Master, tema e brilho permanecem globais. Aceita V016 ou SMKTRON (_451/_452/_453/_454), sem instalar V016 antes. Confira v0.3.0-ap8 na inicialização.', 'Acordes, HOLD, MIX/PAN e luzes. Aceita V016 ou SMKTRON (_451/_452/_453/_454), sem instalar V016 antes. Confira v0.3.0-ch4 na inicialização. Após instalar, teste primeiro em volume baixo e sem DAW conectada.')
s=s.replace('../downloads/SMKTRON-AP8-fontes.zip','../chords-preview/source.zip').replace('../CREDITOS.md','https://smktron-rf1.padrini.chatgpt.site/CREDITOS.md').replace('https://smktron-rf1.padrini.chatgpt.site/previous-ap3/install/','https://smktron-rf1.padrini.chatgpt.site/previous-ap8/install/').replace('AP3 oficial preservada','AP8 oficial preservada')
s=s.replace('Cenas gravadas somente em dois setores da VM da aplicação; o SAVE de boot mantém seu setor e formato originais.', 'Cenas: 0x9E000/0x9F000. Acordes globais: 0xA0000/0xA1000, duas cópias com CRC. SAVE de boot mantém seu setor e formato. Calibração e regiões de fábrica fora da allowlist.')
# Keep the proven V016 return-file gate and explicit confirmation before any write.
s=s.replace('<footer>','<footer><a href="../manual/">Manual de uso</a>')
(D/'index.html').write_text(s)
(D/'INSTRUCOES.md').write_text('''# CH4 — versão oficial, somente SMK-37 Pro

1. Use Chrome ou Edge no computador e cabo USB de dados. Feche o simulador com MIDI, DAWs, Mupgrade e outros atualizadores.
2. Escolha seu V016 original para habilitar o arquivo de retorno. O instalador valida o arquivo inteiro.
3. Conecte e consulte; confira o modelo e depois confirme a instalação CH4. Não desconecte durante a escrita.
4. Ao concluir, confira v0.3.0-ch4 na tela. Comece com volume baixo.
5. GLOBE até PAD CHORDS (GLOBAL 3/3); K4 MODE ON. REC no pad 9, destino 1–8, toque tríade e solte todas as teclas.
6. Toque o pad e solte: ele deve parar. Ligue HOLD (10): deve sustentar; outro pad substitui; desligar HOLD ou STOP (15) interrompe.
7. Toque melodia junto; teste os quatro timbres (11–14), MIX (K1) e PANs (K2/K3). K4 OFF/pad 16 sai.
8. Reinicie fora de qualquer gravação; confira a restauração dos acordes, tape, mix/pans. Modo/HOLD/REC iniciam desligados.
9. Confira cenas, timbres normais, ARP e SEQ. O último preset carregado deve pulsar em verde distinto.

Acordes gravados no simulador não são transferidos automaticamente. Até seis notas por acorde e seis vozes por grupo. FX compartilhados: retornos podem ocupar ambos os canais.

Retorno AP8 oficial: https://smktron-rf1.padrini.chatgpt.site/previous-ap8/install/
Este instalador também permite retorno ao V016 com o arquivo original selecionado. Não existe instalação automática ao abrir/conectar; a escrita exige sua confirmação.

Se uma atualização não concluir, mantenha o cabo conectado, preserve o log e consulte a identidade antes de tentar outro procedimento. A identificação USB _453 é compartilhada pelas versões; confira CH4 na tela e o hash do pacote, não apenas o número USB.
''')
meta={'version':'0.3.0-ch4','status':'official-user-authorized','product':'SMK-37 Pro_453','firmware_sha256':sha,'firmware_bytes':len(raw),'app_bytes':615272,'app_limit':619520,'app_free':4248,'hardware_tested':False,'scene_sectors':['0x9E000','0x9F000'],'chord_sectors':['0xA0000','0xA1000'],'boot_sector':'0x9D000','official_rollback':'https://smktron-rf1.padrini.chatgpt.site/previous-ap8/install/','source':'../chords-preview/source.zip'}
(D/'manifest.json').write_text(json.dumps(meta,indent=2)+'\n')
print(json.dumps(meta))
