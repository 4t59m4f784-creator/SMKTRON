from pathlib import Path
import shutil,re,json,hashlib,zipfile
R=Path(__file__).resolve().parents[2];D=R/'dist/zoom-preview';I=R/'dist/install-zoom';O=R.parent/'smktron-official/dist'
for n in ['index.html','app.js','worklet.js','style.css','credits.css']:
 s=(O/n).read_text().replace('0.3.0-at2','0.3.0-zm1').replace('official-at2','zoom-zm1').replace('SMKTRON · AT2','SMKTRON · ZM1').replace('downloads/SMKTRON-AT2-fontes.zip','source.zip')
 if n=='index.html':
  s=s.replace('href="/"','href="../zoom-preview/"').replace('href="manual/"','href="../manual/"').replace('href="install/"','href="../install-zoom/"').replace('href="CREDITOS.md"','href="https://smktron-rf1.padrini.chatgpt.site/CREDITOS.md"')
  s=s.replace('<main>','<main><p style="font-size:13px;line-height:1.6">ZM1 · Tema Zoom em teste. GLOBE duas vezes → SYSTEM → K2 até ZOOM. SAVE guarda a preferência global. <a href="REPORT.md">Instruções e validação</a> · <a href="../install-zoom/">Instalar no teclado</a></p>',1)
 if n=='app.js':
  a=s.index('const STORE=');b=s.index('const PAGES=',a)
  s=s[:a]+'''const STORE='smktron-zoom-zm1-boot-v1',SCSTORE='smktron-zoom-zm1-scenes-v1',CHSTORE='smktron-zoom-zm1-chords-v1';
try{if(!localStorage.getItem('smktron-zoom-zm1-imported')){for(const [to,from]of [[STORE,'smktron-scenes-sc1-boot-v1'],[SCSTORE,'smktron-scenes-sc1-slots-v1'],[CHSTORE,'smktron-pad-chords-global-v1']]){const v=localStorage.getItem(from);if(v&&!localStorage.getItem(to))localStorage.setItem(to,v);}localStorage.setItem('smktron-zoom-zm1-imported','1');}}catch{}
'''+s[b:]
 (D/n).write_text(s)
shutil.copy2(R/'experiments/zoom/REPORT.md',D/'REPORT.md')
shutil.copytree(O/'install',I,dirs_exist_ok=True)
for p in (I/'firmware').glob('*'):p.unlink()
f=R/'experiments/chords/build/engine/build/mel-0.3.0-zm1-smk37.fwsc';sha=hashlib.sha256(f.read_bytes()).hexdigest();shutil.copy2(f,I/'firmware/smktron-0.3.0-zm1-pro.fwsc')
old=json.loads((I/'manifest.json').read_text())['firmware_sha256']
for p in I.iterdir():
 if p.is_file() and p.suffix in ['.html','.json','.md','.js']:
  s=p.read_text().replace(old,sha).replace('0.3.0-at2','0.3.0-zm1').replace('audio.at2.pending','zoom.zm1.pending').replace('../downloads/SMKTRON-AT2-fontes.zip','../zoom-preview/source.zip').replace('../previous-pm1/install/','https://smktron-rf1.padrini.chatgpt.site/install/').replace('../credits.css','../zoom-preview/credits.css')
  if p.name=='index.html':
   s=s.replace('AT2 · ESTÁVEL E OFICIAL','ZM1 · ZOOM EM TESTE').replace('SMKTRON — AT2 · Oficial','SMKTRON — ZM1 · Zoom em teste').replace('href="../"','href="../zoom-preview/"').replace('Simulador AT2','Simulador ZM1').replace('PM1 anterior preservada','Voltar à AT2 oficial').replace('PAD CHORDS · 0.3.0-zm1 · oficial','ZOOM · 0.3.0-zm1 · protótipo')
   s=s.replace('Pad Chords.<br>Seu SMKTRON completo.','Zoom.<br>Parâmetros em destaque.')
   s=re.sub(r'<div class="notice">.*?</div>','<div class="notice"><strong>ZM1 — tema Zoom em teste.</strong> Light / Dark / Zoom, salvo globalmente. GLOBE duas vezes até SYSTEM; gire K2 até ZOOM. SAVE breve guarda para o próximo boot. Patch conserva as cores dos tapes; Layer mostra a composição dos dois timbres. Diagnóstico e abertura mantêm o desenho. <a href="../zoom-preview/REPORT.md">Instruções e validação</a>. App com 600 bytes livres. Sem teste físico nesta sessão; AT2 continua oficial.</div>',s,flags=re.S)
  p.write_text(s)
m=json.loads((I/'manifest.json').read_text());m.update(status='private-zoom-test',hardware_tested=False,user_approved=False,app_bytes=618920,app_free=600,source='../zoom-preview/source.zip',official_rollback='https://smktron-rf1.padrini.chatgpt.site/install/');m.pop('approval',None);(I/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
(I/'INSTRUCOES.md').write_text((R/'experiments/zoom/REPORT.md').read_text())
with zipfile.ZipFile(O/'downloads/SMKTRON-AT2-fontes.zip') as src,zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 changed={p.relative_to(R).as_posix():p for folder in ['experiments/chords','experiments/zoom'] for p in (R/folder).iterdir() if p.is_file()}
 for n in src.namelist():
  if n not in changed:z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
with zipfile.ZipFile(D/'source.zip','a',zipfile.ZIP_DEFLATED) as z:
 for folder in [D,I]:
  for p in folder.iterdir():
   if p.is_file() and p.suffix in ['.html','.js','.css','.json','.md']:z.write(p,'zoom-distribution/'+str(p.relative_to(R/'dist')))
print('ZM1 firmware:',sha)
