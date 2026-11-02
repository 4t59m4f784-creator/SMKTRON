from pathlib import Path
import shutil,zipfile,json,hashlib
R=Path(__file__).resolve().parents[2];D=R/'dist/pads-preview';T=R/'dist/chords-preview'
for name in ['index.html','app.js','worklet.js','lab-engine.js','style.css','credits.css']:
 s=(T/name).read_text().replace('v=ch4','v=pm1').replace('0.3.0-ch4','0.3.0-pm1')
 if name=='app.js':s=s.replace("b=await cache(id);", "b=null;").replace("'KNOB BANK','DIAGNÓSTICO'","'KNOB BANK','PAD MODES'")
 if name=='index.html':s=s.replace('SMKTRON · PAD CHORDS CH3','SMKTRON · PADS PM1').replace('CH4 OFICIAL','PADS PM1 · TESTE').replace('<main>','<main><p class="build-hint"><b>PADS PM1 · teste privado.</b> KNOB BANK: Performance → Presets → Chord Pads. GLOBE e SAVE preservados. <a href="../install-pads/">Instalador PM1</a>. A versão oficial CH4 permanece inalterada.</p>',1)
 (D/name).write_text(s)
with zipfile.ZipFile(T/'source.zip') as src,zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 changed={str(p.relative_to(R)):p for p in (R/'experiments/chords').glob('*') if p.is_file() and p.suffix in ['.h','.py','.txt']}
 for n in src.namelist():
  if n not in changed:z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
 z.write(R/'tests/pad-modes.test.mjs','tests/pad-modes.test.mjs')
 z.write(R/'tests/pad-modes-results.txt','tests/pad-modes-results.txt')
 z.write(R/'tests/boot.test.mjs','tests/boot.test.mjs')
 z.write(R/'tests/boot-regression-results.txt','tests/boot-regression-results.txt')
I=R/'dist/install-pads';shutil.copytree(R/'dist/install',I,dirs_exist_ok=True)
for p in (I/'firmware').glob('*'):p.unlink()
f=R/'experiments/chords/build/engine/build/mel-0.3.0-pm1-smk37.fwsc';name='smktron-0.3.0-pm1-pro.fwsc';shutil.copy2(f,I/'firmware'/name)
old=json.loads((I/'manifest.json').read_text())['firmware_sha256'];sha=hashlib.sha256(f.read_bytes()).hexdigest()
for p in I.iterdir():
 if p.is_file() and p.suffix in ['.html','.js','.md','.json']:
  s=p.read_text().replace(old,sha).replace('0.3.0-ch4','0.3.0-pm1').replace('chords.ch4.pending','boot.pm1.pending').replace('CH4 · ESTÁVEL E OFICIAL','PM1 · TESTE DE BOOT').replace('SMKTRON CH4 — versão oficial.','SMKTRON PM1 — teste de modos dos pads.').replace('../chords-preview/','../pads-preview/').replace('Simulador CH4','Simulador PM1').replace('4.248','3.384')
  s=s.replace('PM1 · TESTE DE BOOT','PM1 · MODOS DOS PADS').replace('3.384','2.796').replace('O firmware cabe com 2.796 bytes livres.', 'KNOB BANK curto: Performance → Presets fixos → Chord Pads fixos → Performance. KNOB BANK longo: diagnóstico. GLOBE e SAVE mantêm seus acessos anteriores. Inclui a abertura BT2 aprovada. O firmware cabe com 2.796 bytes livres.')
  p.write_text(s)
m=json.loads((I/'manifest.json').read_text());m.update(status='private-boot-test',app_bytes=616724,app_free=2796,source='../pads-preview/source.zip');(I/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
print('PM1 preview and isolated installer packaged; official CH4 untouched')
