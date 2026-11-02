"""Package GR1 simulator only. Existing Zoom/USB/official routes are untouched."""
from pathlib import Path
import json,re,zipfile
from PIL import Image
import struct
R=Path(__file__).resolve().parents[2];D=R/'dist/grain-preview';Z=R/'dist/zoom-preview';H=R/'experiments/grain'
for name in ['index.html','app.js','worklet.js','style.css','credits.css']:
 s=(Z/name).read_text().replace('0.3.0-zm1','0.3.0-gr1').replace('zoom-zm1','grain-gr1').replace('SMKTRON · ZM1','SMKTRON · GR1').replace('../zoom-preview/','../grain-preview/')
 if name=='index.html':
  s=s.replace('<a href="../install-zoom/">Instalador</a>', '<a href="../zoom-preview/">Zoom sem granulação</a>')
  s=re.sub(r'<main><p style=.*?</p>', '''<main><p class="grain-intro"><strong>GR1 · Fragmentos de fita.</strong> Protótipo privado. O botão GRAIN (ARP no teclado) abre SIZE / SPREAD / DENSITY / MIX. Começa com MIX em 0%; toque uma nota ou acorde e experimente os ajustes abaixo. O sequenciador continua disponível. <a href="REPORT.md">Notas do teste</a></p>
<div class="grain-tools" role="group" aria-label="Exemplos de granulação"><span>EXPERIMENTE</span><button data-grain="original">Fita original</button><button data-grain="bruma">Bruma</button><button data-grain="pulsos">Pulsos</button><button data-grain="fragmentos">Fragmentos</button></div>
<p class="grain-help">K1: tamanho (20–400 ms) · K2: dispersão da posição · K3: densidade · K4: mistura. Os ajustes GRAIN valem para esta sessão; SAVE mantém os demais parâmetros como antes.</p>''',s,count=1,flags=re.S)
 if name=='app.js':
  a=s.index('const STORE=');b=s.index('const PAGES=',a)
  s=s[:a]+"const STORE='smktron-grain-gr1-boot-v1',SCSTORE='smktron-grain-gr1-scenes-v1',CHSTORE='smktron-grain-gr1-chords-v1';\n"+s[b:]
  s=s.replace("'SETUP','ARP','SEQ'","'SETUP','GRAIN','SEQ'").replace("[6,'ARP','ON / OFF']","[6,'GRAIN','FRAGMENTOS']").replace('["MODE", "RATE", "OCTAVES", "GATE"]','["SIZE", "SPREAD", "DENSITY", "MIX"]')
  s+='''
const grainPresets={bruma:[200,65,100,75],pulsos:[70,25,35,85],fragmentos:[35,100,85,100]};
for(const b of $$('[data-grain]'))b.onclick=()=>{
 if(!ready){status('Inicie o áudio antes de escolher um exemplo.');return;}
 send({type:'button',id:6,down:true});setTimeout(()=>send({type:'button',id:6,down:false}),70);
 const p=grainPresets[b.dataset.grain];if(p)p.forEach((value,k)=>send({type:'grain',k,value}));else send({type:'grain',k:3,value:0});
 status(b.dataset.grain==='original'?'Fita original · MIX 0%':'GRAIN · '+b.textContent+' · toque notas ou um acorde');
};
'''
 if name=='worklet.js':s=s.replace("case 'key':", "case 'grain':e.web_grain_set(m.k,m.value);break;\n   case 'key':")
 if name=='style.css':s+='\n.grain-intro,.grain-help{font-size:13px;line-height:1.6}.grain-tools{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin:14px 0}.grain-tools>span{font-size:11px;letter-spacing:.12em;color:#596b60;margin-right:6px}.grain-tools button{padding:9px 14px}.grain-help{color:#596b60;margin:8px 0 18px}\n'
 (D/name).write_text(s)
for p in (H/'captures').glob('*.rgb565'):
 pixels=struct.unpack('<57600H',p.read_bytes());im=Image.new('RGB',(240,240));im.putdata([(((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31) for c in pixels]);im.resize((720,720),Image.Resampling.NEAREST).save(D/(p.stem+'.png'))
if (H/'REPORT.md').exists():(D/'REPORT.md').write_text((H/'REPORT.md').read_text())
with zipfile.ZipFile(Z/'source.zip') as src,zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 changed={p.relative_to(R).as_posix():p for folder in ['experiments/grain','experiments/chords'] for p in (R/folder).iterdir() if p.is_file()}
 for n in src.namelist():
  if n not in changed:z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
 for p in D.iterdir():
  if p.is_file() and p.suffix in ['.html','.js','.css','.md']:z.write(p,'grain-distribution/'+p.name)
print('Packaged GR1 simulator (no hardware installer).')
