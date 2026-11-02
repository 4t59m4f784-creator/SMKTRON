from pathlib import Path
import shutil,hashlib,json,re,zipfile
R=Path(__file__).resolve().parents[2]; I=R/'dist/install-grain3'; D=R/'dist/grain3-preview'
shutil.copytree(R/'dist/install-zoom',I,dirs_exist_ok=True)
for p in (I/'firmware').iterdir():p.unlink()
f=R/'experiments/chords/build/engine/build/mel-0.3.0-gr3-smk37.fwsc'
sha=hashlib.sha256(f.read_bytes()).hexdigest();shutil.copy2(f,I/'firmware/smktron-0.3.0-gr3-pro.fwsc')
old=json.loads((I/'manifest.json').read_text())['firmware_sha256']
for p in I.iterdir():
 if p.is_file() and p.suffix in ['.html','.js','.json','.md']:
  s=p.read_text().replace(old,sha).replace('0.3.0-zm1','0.3.0-gr3').replace('zoom.zm1.pending','grain.gr3.pending').replace('../zoom-preview/','../grain3-preview/').replace('ZM1','GR3').replace('ZOOM EM TESTE','GRAIN EM TESTE').replace('Zoom em teste','Grain em teste').replace('ZOOM ·','GRAIN ·')
  if p.name=='index.html':
   s=s.replace('Zoom.<br>Parâmetros em destaque.','GRAIN.<br>Fragmentos de fita.')
   s=re.sub(r'<div class="notice">.*?</div>','<div class="notice"><strong>GR3 — protótipo aprovado no simulador.</strong> GRAIN substitui o arpejador; o sequenciador permanece. Botão ARP abre GRAIN; K6 escolhe o estilo e o pad 10 liga/desliga em Performance. A roda chama VIBRATO e controla seu mix. Light, Dark e Zoom disponíveis. App com 33.488 bytes livres. Validação física pendente. <a href="../grain3-preview/REPORT.md">Instruções e validação</a>.</div>',s,flags=re.S)
   s=s.replace('SAVE de boot mantém seu setor e formato.','SAVE de boot mantém seu setor; registros migram para v7. A AT2 não lê os novos registros v7.')
  p.write_text(s)
m=json.loads((I/'manifest.json').read_text());m.update(status='private-grain-hardware-prototype',app_bytes=586032,app_free=33488,hardware_tested=False,simulator_user_approved=True,source='../grain3-preview/source.zip');m.pop('user_approved',None)
(I/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
report=(R/'experiments/grain3/REPORT.md').read_text().replace('Não há teste físico nem instalador GR3 publicado nesta etapa.','Instalador GR3 disponível em ../install-grain3/. Build nativo validado; teste físico pendente. O usuário aprovou o simulador.')
report+='\n## Instalação GR3\n\nUse Chrome/Edge no computador e USB de dados. Separe o V016 original para a conferência do retorno. Feche DAWs e o simulador antes de conectar. Confira 0.3.0-gr3 no boot. Teste GRAIN (ARP), K6, pad 10, roda, SEQ, favoritos e Boot SAVE nos três temas. Os registros v6 existentes são migrados ao carregar; novos salvamentos usam v7. A AT2 não entende v7; antes de salvar nesta versão, anote os ajustes que deseja manter ao retornar. Não inclui novos samples.\n'
(D/'REPORT.md').write_text(report);(I/'INSTRUCOES.md').write_text(report)
p=D/'index.html';s=p.read_text().replace('<a href="../zoom-preview/">Zoom sem granulação</a>','<a href="../install-grain3/">Instalador GR3</a>');p.write_text(s)
zpath=D/'source.zip';tmp=D/'source.tmp.zip'
with zipfile.ZipFile(zpath) as src,zipfile.ZipFile(tmp,'w',zipfile.ZIP_DEFLATED) as z:
 changed={'experiments/grain3/package_installer.py':Path(__file__),'experiments/grain3/REPORT.md':D/'REPORT.md'}
 for n in src.namelist():
  if n not in changed and not n.startswith('grain3-installer/'):z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
 for p in I.iterdir():
  if p.is_file():z.write(p,'grain3-installer/'+p.name)
tmp.replace(zpath)
print('GR3 firmware SHA256',sha)
