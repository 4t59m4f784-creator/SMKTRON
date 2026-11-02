"""Package GR2 simulator only. Existing Zoom/USB/official routes are untouched."""
from pathlib import Path
import json,re,zipfile
from PIL import Image
import struct
R=Path(__file__).resolve().parents[2];D=R/'dist/grain2-preview';Z=R/'dist/zoom-preview';H=R/'experiments/grain2'
for name in ['index.html','app.js','worklet.js','style.css','credits.css']:
 s=(Z/name).read_text().replace('0.3.0-zm1','0.3.0-gr2').replace('zoom-zm1','grain-gr2').replace('SMKTRON · ZM1','SMKTRON · GR2').replace('../zoom-preview/','../grain2-preview/')
 if name=='index.html':
  s=s.replace('<a href="../install-zoom/">Instalador</a>', '<a href="../zoom-preview/">Zoom sem granulação</a>')
  s=re.sub(r'<main><p style=.*?</p>', '''<main><p class="grain-intro"><strong>GR2 · Fragmentos de fita.</strong> Protótipo privado. O botão GRAIN (ARP no teclado) abre SIZE / SPREAD / DENSITY / MIX. K6 escolhe FREE / BRUMA / PULSOS / FRAGMENTOS. O pad 10 liga GRAIN; a roda de modulação abre VIBRATO. O sequenciador continua disponível. <a href="REPORT.md">Notas do teste</a></p>
<div class="grain-tools" role="group" aria-label="Exemplos de granulação"><span>EXPERIMENTE</span><button data-grain="original">Fita original</button><button data-grain="bruma">Bruma</button><button data-grain="pulsos">Pulsos</button><button data-grain="fragmentos">Fragmentos</button></div>
<p class="grain-help">K1: tamanho (20–400 ms) · K2: dispersão da posição · K3: densidade · K4: mistura. GRAIN é salvo no boot e nos favoritos. Em VIBRATO, K4 define MAX MIX; a roda controla a mistura até esse limite.</p>''',s,count=1,flags=re.S)
 if name=='app.js':
  a=s.index('const STORE=');b=s.index('const PAGES=',a)
  s=s[:a]+"const STORE='smktron-grain-gr2-boot-v1',SCSTORE='smktron-grain-gr2-scenes-v1',CHSTORE='smktron-grain-gr2-chords-v1';\n"+s[b:]
  s=s.replace("'SETUP','ARP','SEQ'","'SETUP','GRAIN','SEQ'").replace("[6,'ARP','ON / OFF']","[6,'GRAIN','FRAGMENTOS']").replace('["MODE", "RATE", "OCTAVES", "GATE"]','["SIZE", "SPREAD", "DENSITY", "MIX"]').replace('["RATE", "DEPTH", "TONE", "MIX"]','["RATE", "DEPTH", "TONE", "MAX MIX"]').replace("'LOFI','VIBRATO','LESLIE'","'LOFI','GRAIN','LESLIE'")
  s+='''
const grainPresets={bruma:[200,65,100,75],pulsos:[70,25,35,85],fragmentos:[35,100,85,100]};
for(const b of $$('[data-grain]'))b.onclick=()=>{
 if(!ready){status('Inicie o áudio antes de escolher um exemplo.');return;}
 send({type:'button',id:6,down:true});setTimeout(()=>send({type:'button',id:6,down:false}),70);
 const style={bruma:1,pulsos:2,fragmentos:3}[b.dataset.grain];if(style)send({type:'grain-style',value:style});send({type:'grain-on',value:!!style});
 status(b.dataset.grain==='original'?'Fita original · GRAIN OFF':'GRAIN · '+b.textContent+' · toque notas ou um acorde');
};
'''
 if name=='worklet.js':s=s.replace("case 'key':", "case 'grain':e.web_grain_set(m.k,m.value);break;\n   case 'grain-style':e.web_grain_style(m.value);break;\n   case 'grain-on':e.web_grain_on(m.value?1:0);break;\n   case 'key':")
 if name=='style.css':s+='\n.grain-intro,.grain-help{font-size:13px;line-height:1.6}.grain-tools{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin:14px 0}.grain-tools>span{font-size:11px;letter-spacing:.12em;color:#596b60;margin-right:6px}.grain-tools button{padding:9px 14px}.grain-help{color:#596b60;margin:8px 0 18px}\n'
 if name=='worklet.js':s=s.replace("msg.bank=e.web_fader_bank();","msg.grain=[e.web_grain_state(0),e.web_grain_state(1)];msg.bank=e.web_fader_bank();")
 if name=='app.js':
  s=s.replace("[6,m.state[4]]","[6,m.grain?.[0]||0]").replace("if(m.state){const pg=m.state[0];","if(m.state){const pg=m.state[0];const k6=document.querySelector('[data-knob=\"5\"]');k6.querySelector('label').textContent=pg===10?'K6 · STYLE':'K6 · PÁGINAS';k6.querySelector('.knob').setAttribute('aria-label',pg===10?'K6 · STYLE':'K6 · PÁGINAS');")
 (D/name).write_text(s)
for p in (H/'captures').glob('*.rgb565'):
 pixels=struct.unpack('<57600H',p.read_bytes());im=Image.new('RGB',(240,240));im.putdata([(((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31) for c in pixels]);im.resize((720,720),Image.Resampling.NEAREST).save(D/(p.stem+'.png'))
if (H/'REPORT.md').exists():(D/'REPORT.md').write_text((H/'REPORT.md').read_text())
with zipfile.ZipFile(Z/'source.zip') as src,zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 changed={p.relative_to(R).as_posix():p for folder in ['experiments/grain','experiments/grain2','experiments/chords'] for p in (R/folder).iterdir() if p.is_file()}
 for n in src.namelist():
  if n not in changed:z.writestr(n,src.read(n))
 for n,p in changed.items():z.write(p,n)
 for p in D.iterdir():
  if p.is_file() and p.suffix in ['.html','.js','.css','.md']:z.write(p,'grain-distribution/'+p.name)
print('Packaged GR2 simulator (no hardware installer).')
