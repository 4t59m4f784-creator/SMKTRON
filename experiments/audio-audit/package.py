from pathlib import Path
import shutil,json,hashlib,zipfile
R=Path(__file__).resolve().parents[2];D=R/'dist/audio-preview';T=R/'dist/pads-preview';I=R/'dist/install-audio'
for n in ['index.html','app.js','worklet.js','style.css','credits.css']:
 s=(T/n).read_text().replace('0.3.0-pm1','0.3.0-at2').replace('official-pm1','audio-at2').replace('../install-pads/','../install-audio/')
 if n=='index.html':s=s.replace('<main>','<main><p style="font-size:12px">AT2 · Favoritos nos pads, display Patch/Layer e transição suave. <a href="REPORT.md">Relatório técnico</a>. PM1 oficial preservada.</p>',1)
 (D/n).write_text(s)
shutil.copy2(R/'experiments/audio-audit/REPORT.md',D/'REPORT.md')
with zipfile.ZipFile(T/'source.zip') as src,zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 changed={p.relative_to(R).as_posix():p for folder in ['experiments/chords','experiments/audio-audit'] for p in (R/folder).iterdir() if p.is_file()}
 for n in src.namelist():
  if n not in changed:z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
shutil.copytree(R/'dist/install-pads',I,dirs_exist_ok=True)
for p in (I/'firmware').glob('*'):p.unlink()
f=R/'experiments/chords/build/engine/build/mel-0.3.0-at2-smk37.fwsc';sha=hashlib.sha256(f.read_bytes()).hexdigest();shutil.copy2(f,I/'firmware/smktron-0.3.0-at2-pro.fwsc')
old=json.loads((I/'manifest.json').read_text())['firmware_sha256']
for p in I.iterdir():
 if p.is_file() and p.suffix in ['.js','.html','.md','.json']:
  s=p.read_text().replace(old,sha).replace('0.3.0-pm1','0.3.0-at2').replace('boot.pm1.pending','audio.at2.pending').replace('../pads-preview/','../audio-preview/').replace('PM1 · ESTÁVEL E OFICIAL','AT2 · TESTE DE ÁUDIO').replace('SMKTRON PM1 — versão oficial.','SMKTRON AT2 — suavização de áudio.').replace('Simulador PM1','Simulador AT2').replace('2.796','3.500')
  if p.suffix=='.html':s=s.replace('<div class="notice">','<div class="notice"><b>Teste isolado: rampas de reverb/volume, histórico de vibrato e transição curta entre favoritos. Não resolve interrupções de gravação na flash. Sem novo teste físico nesta sessão. <a href="../audio-preview/REPORT.md">Relatório</a>.</b> ',1)
  p.write_text(s)
p=I/'manifest.json';m=json.loads(p.read_text());m.update(status='private-audio-test',hardware_tested=False,app_bytes=616020,app_free=3500,source='../audio-preview/source.zip');p.write_text(json.dumps(m,indent=2)+'\n')
print('AT2 preview/installer packaged; PM1 unchanged')
