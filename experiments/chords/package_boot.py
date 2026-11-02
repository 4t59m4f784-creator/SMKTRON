from pathlib import Path
import shutil,zipfile,json,hashlib
R=Path(__file__).resolve().parents[2];D=R/'dist/boot-preview';T=R/'dist/chords-preview'
for name in ['index.html','app.js','worklet.js','lab-engine.js','style.css','credits.css']:
 s=(T/name).read_text().replace('v=ch4','v=bt2').replace('0.3.0-ch4','0.3.0-bt2')
 if name=='app.js':s=s.replace("b=await cache(id);", "b=null;")
 if name=='index.html':s=s.replace('SMKTRON · PAD CHORDS CH3','SMKTRON · BOOT BT2').replace('CH4 OFICIAL','BOOT BT2 · TESTE').replace('<main>','<main><p class="build-hint"><b>BOOT BT2 · teste privado.</b> Abertura procedural e pads sincronizados. <a href="../install-boot/">Instalador BT2</a>. A versão oficial CH4 permanece inalterada.</p>',1)
 (D/name).write_text(s)
with zipfile.ZipFile(T/'source.zip') as src,zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 changed={str(p.relative_to(R)):p for p in (R/'experiments/chords').glob('*') if p.is_file() and p.suffix in ['.h','.py','.txt']}
 for n in src.namelist():
  if n not in changed:z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
 z.write(R/'tests/boot.test.mjs','tests/boot.test.mjs')
 z.write(R/'tests/boot-regression-results.txt','tests/boot-regression-results.txt')
I=R/'dist/install-boot';shutil.copytree(R/'dist/install',I,dirs_exist_ok=True)
for p in (I/'firmware').glob('*'):p.unlink()
f=R/'experiments/chords/build/engine/build/mel-0.3.0-bt2-smk37.fwsc';name='smktron-0.3.0-bt2-pro.fwsc';shutil.copy2(f,I/'firmware'/name)
old=json.loads((I/'manifest.json').read_text())['firmware_sha256'];sha=hashlib.sha256(f.read_bytes()).hexdigest()
for p in I.iterdir():
 if p.is_file() and p.suffix in ['.html','.js','.md','.json']:
  s=p.read_text().replace(old,sha).replace('0.3.0-ch4','0.3.0-bt2').replace('chords.ch4.pending','boot.bt2.pending').replace('CH4 · ESTÁVEL E OFICIAL','BT2 · TESTE DE BOOT').replace('SMKTRON CH4 — versão oficial.','SMKTRON BT2 — teste de abertura.').replace('../chords-preview/','../boot-preview/').replace('Simulador CH4','Simulador BT2').replace('4.248','3.384')
  p.write_text(s)
m=json.loads((I/'manifest.json').read_text());m.update(status='private-boot-test',app_bytes=616136,app_free=3384,source='../boot-preview/source.zip');(I/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
print('BT2 preview and isolated installer packaged; official CH4 untouched')
