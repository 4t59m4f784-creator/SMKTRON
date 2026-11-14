"""Isolated simulator + installer. Never edits official or GR3 routes."""
from pathlib import Path
import shutil,re,json,hashlib,zipfile,struct
from PIL import Image
R=Path(__file__).resolve().parents[2];H=Path(__file__).parent;D=R/'dist/midi-preview';I=R/'dist/install-midi'
for n in ['index.html','app.js','worklet.js','style.css','credits.css']:
 s=(R/'dist/grain3-preview'/n).read_text().replace('grain3-preview','midi-preview').replace('install-grain3','install-midi').replace('0.3.0-gr3','0.3.0-mm1f').replace('grain-gr3','midi-mm1')
 if n=='index.html':
  s=re.sub(r'<main>.*?<div class="session-bar">','<main><p data-pt="Protótipo MIDI MODE · Instrumento e controlador independentes." data-en="MIDI MODE prototype · Independent instrument and controller modes.">Protótipo MIDI MODE · Instrumento e controlador independentes.</p><div class="mode-actions"><button id="modeSynth">SMKTRON</button><button id="modeMidi">MIDI MODE</button><a href="REPORT.md">PT / EN · Guia / Guide</a><select id="language"><option value="pt">Português</option><option value="en">English</option></select></div><div class="session-bar">',s,count=1,flags=re.S)
  s=s.replace('Instalador GR3','Instalar / Install').replace('SMKTRON · GR3','SMKTRON · MM1f').replace('Intensidade fixa','Velocity / MIDI MODE')
  s=s.replace('<select id="midiInput"','<select id="midiOutput" aria-label="MIDI output"><option value="">MIDI OUT: monitor</option></select><select id="midiInput"')
  s=s.replace('<span id="padModeLabel">','<button id="padBankMidi" hidden>PADS A / B</button><span id="padModeLabel">')
  s=s.replace('Conectar MIDI','Conectar MIDI / Connect MIDI').replace('href="../manual/"','href="https://4t59m4f784-creator.github.io/SMKTRON/manual/"')
  s=s.replace('<h2>Código e projetos de origem</h2>','<p>MIDI MODE: SMKSYNTH MM1a, adaptado de Snoop37 / M. Geisler. Integração SMKTRON: Marcus Padrini e ChatGPT/Codex.</p><h2>Código e projetos de origem</h2>')
  s=s.replace('</body>','<script type="module" src="i18n.js"></script></body>')
 if n=='app.js':
  a=s.index('try{if(!localStorage');b=s.index('const PAGES=',a);s=s[:a]+s[b:]
  s=s.replace("smktron-grain-gr3",'smktron-midi-mm1').replace("'PAD CHORDS'];","'PAD CHORDS','MIDI MODE'];")
  s=s.replace("[3,'K4 · PARÂMETRO 4'],","[3,'K4 · PARÂMETRO 4'],[4,'K5 · CONTROLE'],")
  s=s.replace('[[12,\'PATCH\'','[[12,\'PATCH\'')
  s=s.replace('["MIX","PAN MELODY","PAN CHORDS","MODE"]];','["MIX","PAN MELODY","PAN CHORDS","MODE"],["MIDI MODE","—","—","—"]];')
  s=s.replace("if(m.type==='error')", "if(m.type==='midi-out'){for(const data of m.data){const port=midiAccess?.outputs.get($('#midiOutput').value);if(port)port.send(data)}return;}\n if(m.type==='error')",1)
  s=s.replace("if(m.type!=='frame')return;","if(m.type!=='frame')return;\n if(m.maps){try{localStorage.setItem('smktron-midi-mm1-maps',JSON.stringify(m.maps))}catch{}}")
  s=s.replace("PARAMS[pg][i]","(m.mm?.[0]?(m.mm[6]?['SYNTH','MAP','CHANNEL','ABS / REL']:m.mm[5]?['TYPE','NUMBER','CHANNEL','COLOUR']:['MIDI CC','MIDI CC','MIDI CC','MIDI CC']):PARAMS[pg])[i]")
  # Override synth-only pad captions and enable all controller pads after the normal caption update.
  s=s.replace("\n}\n$('#start').onclick",'''\n if(m.mm){const on=!!m.mm[0];$('#screenState').textContent=on?'MIDI MODE · MAP '+(m.mm[1]+1)+' · CH '+m.mm[2]:$('#screenState').textContent;$('#modeMidi').setAttribute('aria-pressed',on);$('#modeSynth').setAttribute('aria-pressed',!on);$('#padBankMidi').hidden=!on;document.querySelector('[data-knob="4"]').hidden=!on;$('#volume').disabled=on;
 if(on){$('#padModeLabel').textContent='MIDI / '+(m.mm[9]?'B':'A');$$('.pad').forEach((b,i)=>{b.disabled=false;b.querySelector('small').textContent='PAD '+(m.mm[9]?'B':'A')+(i+1)});$$('.fader').forEach((f,i)=>f.querySelector('small').textContent='F'+(m.mm[8]?'B':'A')+(i+1));$('#faderBank').textContent='FADERS '+(m.mm[8]?'B':'A');}}
}\n$('#start').onclick''')
  s=s.replace("node.port.postMessage({type:'load',wasm,settings,sceneStore,chordStore});","let maps;try{maps=JSON.parse(localStorage.getItem('smktron-midi-mm1-maps'))}catch{}\n  node.port.postMessage({type:'load',wasm,settings,sceneStore,chordStore,maps});")
  s=s.replace("send({type:'midi',data:[0xB0,64,e.target.checked?127:0]})","send({type:'pedal',value:e.target.checked?1023:0})")
  s=s.replace("releaseAll();await ctx.suspend();","releaseAll();send({type:'panic'});await new Promise(resolve=>setTimeout(resolve,120));await ctx.suspend();")
  a=s.index('const grainPresets=');s=s[:a]
  s+='''
$('#modeMidi').onclick=()=>{releaseAll();send({type:'mode',value:1})};
$('#modeSynth').onclick=()=>{releaseAll();send({type:'mode',value:0})};
$('#padBankMidi').onclick=()=>send({type:'pad-bank'});
$('#midiOutput').onchange=()=>{send({type:'panic'});};
const fillOutputs=()=>{const old=$('#midiOutput').value;$('#midiOutput').innerHTML='<option value="">MIDI OUT: monitor</option>';for(const p of midiAccess?.outputs.values()||[]){const o=document.createElement('option');o.value=p.id;o.textContent=p.name;$('#midiOutput').append(o)}if(midiAccess?.outputs.has(old))$('#midiOutput').value=old;};
midiList=()=>{if(midiPort)midiPort.onmidimessage=null;$('#midiInput').hidden=true;fillOutputs();status('MIDI OUT: escolha uma saída / choose an output');};
let previousMidiOutput='';$('#midiOutput').onchange=()=>{const old=midiAccess?.outputs.get(previousMidiOutput);if(old){for(let c=0;c<16;c++){old.send([0xB0|c,64,0]);old.send([0xB0|c,123,0]);old.send([0xB0|c,120,0]);}}previousMidiOutput=$('#midiOutput').value;send({type:'panic'});};
'''
 if n=='worklet.js':
  s=s.replace('this.chordWas=0;','this.chordWas=0;this.mapWas=0;')
  s=s.replace('this.ex.web_master(700);',"if(m.maps?.length===8192)new Uint8Array(this.ex.memory.buffer,this.ex.web_mm_flash(),8192).set(m.maps);\n    this.ex.web_master(700);")
  s=s.replace("case 'panic':e.web_panic();break;","case 'panic':e.web_mm_panic();break;\n   case 'mode':e.web_midi_mode(m.value);break;\n   case 'pedal':e.web_mm_pedal(m.value);break;\n   case 'pad-bank':e.web_button(0,1);e.web_button(1,1);e.web_render(128);e.web_button(0,0);e.web_button(1,0);break;")
  s=s.replace("case 'pad-cancel':if(e.web_chord_state(0))","case 'pad-cancel':if(e.web_mm_state(0))e.web_pad(m.k,0);else if(e.web_chord_state(0))")
  s=s.replace("case 'save-cancel':e.web_scene_cancel_save();", "case 'save-cancel':if(e.web_mm_state(0))e.web_button(11,0);else e.web_scene_cancel_save();")
  s=s.replace('if(currentTime-this.last>1/25)',"const midi=[];while(e.web_mm_tx_count()){const n=e.web_mm_tx_take();midi.push([n&255,n>>8&255,n>>16&255])}if(midi.length)this.port.postMessage({type:'midi-out',data:midi});\n   if(currentTime-this.last>1/25)")
  s=s.replace("msg.grain=", "msg.mm=Array.from({length:13},(_,i)=>e.web_mm_state(i));if(this.mapWas!==e.web_mm_writes()){this.mapWas=e.web_mm_writes();msg.maps=Array.from(new Uint8Array(e.memory.buffer,e.web_mm_flash(),8192));}\n    msg.grain=")
 if n=='style.css':s+='\n.mode-actions{display:flex;align-items:center;gap:12px;flex-wrap:wrap;margin:18px 0}.mode-actions [aria-pressed=true]{background:#267d70;color:white}[hidden]{display:none!important}.knobs:has([data-knob="4"]:not([hidden])){grid-template-columns:repeat(8,minmax(0,1fr))}@media(max-width:800px){.knobs:has([data-knob="4"]:not([hidden])){grid-template-columns:repeat(4,minmax(0,1fr))}}\n'
 (D/n).write_text(s)
shutil.copy2(H/'i18n.js',D/'i18n.js')
shutil.copy2(H/'REPORT.md',D/'REPORT.md')
images=[]
for t in range(3):
 im=Image.new('RGB',(240,240));im.putdata([((v>>11)*255//31,((v>>5)&63)*255//63,(v&31)*255//31)for(v,)in struct.iter_unpack('<H',(H/f'captures/midi-{t}.rgb565').read_bytes())]);im.resize((720,720),Image.Resampling.NEAREST).save(D/f'midi-{t}.png');images.append(im.resize((480,480),Image.Resampling.NEAREST))
im=Image.new('RGB',(1440,480))
for t,p in enumerate(images):im.paste(p,(t*480,0))
im.save(D/'preview.png')
# Clone the proven GR3 updater, with separate identity/hash/resume key and official GR3 return.
shutil.copytree(R/'dist/install-grain3',I,dirs_exist_ok=True)
for p in (I/'firmware').iterdir():p.unlink()
fw=H/'build/engine/build/mel-0.3.0-mm1f-smk37.fwsc';sha=hashlib.sha256(fw.read_bytes()).hexdigest();shutil.copy2(fw,I/'firmware/smktron-0.3.0-mm1f-pro.fwsc')
old=json.loads((I/'manifest.json').read_text())['firmware_sha256']
for p in I.iterdir():
 if p.is_file() and p.suffix in ['.html','.js','.json','.md']:
  s=p.read_text().replace(old,sha).replace('0.3.0-gr3','0.3.0-mm1f').replace('SMK-37 Pro_453','SMK-37 Pro_460').replace('grain.gr3.pending','midi.mm1f.pending').replace('grain3-preview','midi-preview').replace('GR3','MM1').replace('GRAIN EM TESTE','MIDI MODE EM TESTE').replace('Grain em teste','MIDI Mode em teste')
  if p.name=='service.js':s=s.replace("'SMK-37 Pro_454'","'SMK-37 Pro_454','SMK-37 Pro_453','SMK-37 Pro_455','SMK-37 Pro_456','SMK-37 Pro_457','SMK-37 Pro_458','SMK-37 Pro_459'")
  if p.name=='controller.js':s=s.replace("kind==='rc'?453:16","kind==='rc'?460:16")
  if p.name=='index.html':
   s=s.replace('GRAIN.<br>Fragmentos de fita.','SMKTRON.<br>MIDI MODE.')
   s=re.sub(r'<div class="notice">.*?</div>','<div class="notice"><strong>MM1f · MIDI MODE experimental. USB e TRS aprovados no teclado. Telas MIDI ampliadas e leitura exclusiva do K5 no controlador.</strong> GLOBAL → MIDI MODE → K1 direita entra no controlador; GLOBE e K1 esquerda retornam ao instrumento. Quatro mapas, USB/TRS, ABS/REL, cores e edição no teclado. Sem MIDI musical no synth; sem áudio no controlador. <a href="../midi-preview/REPORT.md">Guia PT / EN</a>. GLOBAL e edição com quatro blocos de knobs e suporte ao layout Zoom.</div>',s,flags=re.S)
   s=s.replace('GRAIN · 0.3.0-mm1f · protótipo','MIDI MODE · 0.3.0-mm1f · protótipo').replace('Voltar à AT2 oficial','Instalar a oficial GR3').replace('https://smktron-rf1.padrini.chatgpt.site/install/','./official/')
  p.write_text(s)
# A pinned GR3 package can restore directly from MM1 without stock detour.
O=I/'official';shutil.copytree(I,O,dirs_exist_ok=True,ignore=shutil.ignore_patterns('official','firmware'));(O/'firmware').mkdir(exist_ok=True)
grfw=R/'dist/install-grain3/firmware/smktron-0.3.0-gr3-pro.fwsc';grsha=hashlib.sha256(grfw.read_bytes()).hexdigest();shutil.copy2(grfw,O/'firmware/smktron-0.3.0-gr3-pro.fwsc')
for p in O.iterdir():
 if p.is_file() and p.suffix in ['.html','.js','.json','.md']:
  s=p.read_text().replace(sha,grsha).replace('0.3.0-mm1f','0.3.0-gr3').replace("product:'SMK-37 Pro_460'","product:'SMK-37 Pro_453'").replace('midi.mm1f.pending','midi.return-gr3.pending').replace("kind==='rc'?460:16","kind==='rc'?453:16").replace('SMK-37 Pro_460. Confira','SMK-37 Pro_453. Confira').replace('../midi-preview/','../../midi-preview/').replace('MIDI MODE EM TESTE','RETORNO À GR3').replace('MIDI MODE · 0.3.0-gr3 · protótipo','SMKTRON GR3 · oficial')
  if p.name=='index.html':s=re.sub(r'<div class="notice">.*?</div>','<div class="notice">Retorno ao SMKTRON GR3 oficial. Seus mapas MIDI permanecem guardados, mas a GR3 não oferece MIDI MODE. Cenas, favoritos, acordes e boot mantêm o formato.</div>',s,flags=re.S).replace('href="./official/"','href="../"').replace('href="../manual/"','href="https://4t59m4f784-creator.github.io/SMKTRON/manual/"')
  p.write_text(s)
manifest={'version':'0.3.0-mm1f','product':'SMK-37 Pro_460','firmware_sha256':sha,'firmware_bytes':fw.stat().st_size,'app_bytes':len((H/'build/engine/build/mel.bin').read_bytes()),'app_limit':619520,'hardware_tested':False,'midi_transport_hardware_tested':True,'midi_transport_base':'MM1d / 458','midi_sectors':['0xA2000','0xA3000'],'base':'GR3 / MM1a controller','source':'../midi-preview/source.zip','rollback':'./official/'}
manifest['app_free']=619520-manifest['app_bytes'];(I/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');(O/'manifest.json').write_text(json.dumps({**manifest,'version':'0.3.0-gr3','product':'SMK-37 Pro_453','firmware_sha256':grsha,'app_bytes':586032,'app_free':33488},indent=2)+'\n')
# Corresponding complete build tree, overlays, tests and credit history.
with zipfile.ZipFile(D/'source.zip','w',zipfile.ZIP_DEFLATED) as z:
 for p in (H/'build/engine').rglob('*'):
  if p.is_file() and 'build' not in p.relative_to(H/'build/engine').parts:z.write(p,'engine/'+p.relative_to(H/'build/engine').as_posix())
 for p in H.rglob('*'):
  if p.is_file() and 'build' not in p.relative_to(H).parts and '__pycache__' not in p.parts:z.write(p,'integration/'+p.relative_to(H).as_posix())
 for p in D.iterdir():
  if p.is_file() and p.suffix in ['.js','.html','.css','.md']:z.write(p,'simulator/'+p.name)
print(json.dumps(manifest,indent=2))
