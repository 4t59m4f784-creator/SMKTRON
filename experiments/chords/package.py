from pathlib import Path
import shutil
ROOT=Path(__file__).resolve().parents[2];D=ROOT/'templates/lab';O=ROOT/'dist/chords-preview'
for f in ['style.css','credits.css']:shutil.copy2(D/f,O/f)
html=(D/'index.html').read_text().replace('SMKTRON · Laboratório','SMKTRON · PAD CHORDS CH3').replace('href="/"','href="../"').replace('v=ap8','v=ch4').replace('v=lab1','v=ch4')
html=html.replace('<section class="instrument"','''<section class="chord-help" aria-label="Protótipo Pad Chords"><div><b>PAD CHORDS · CH4 OFICIAL</b><button id="chordMode" aria-pressed="false">Ativar Pad Chords</button></div><p>REC (pad 9) → escolha um pad de 1 a 8 → toque o acorde → solte todas as teclas para gravar. Até 6 notas. HOLD (pad 10) sustenta até outro pad de acorde ou até desligar HOLD.</p><p>Pads 11–14: FLUTE, CHOIR, CELLO, VIOLINS. Pad 15: STOP / cancelar REC. Pad 16: sair. GLOBE três vezes abre GLOBAL 3/3; K4 ativa o modo. K1: MIX (0% só melodia, 100% só acordes); K2: PAN MELODY; K3: PAN CHORDS. Centro: CENTER; L/R: esquerda/direita. Pads de acordes em rosa/lilás; timbres nas cores do SMKTRON. Nos presets, o último carregado pulsa suavemente em verde distinto. Um acorde por vez, seis vozes de teclado + seis de acordes. Mix e pans são globais. Delay e reverb retornam nos dois canais; vibrato e Leslie também permanecem compartilhados. O volume da cena é geral.</p><p><b>SMKTRON:</b> a build CH4 cabe no espaço do teclado, com fontes compactadas sem perda. <a href="../install/">Instalador oficial CH4</a>. Padrão inicial: cena 90%, Master 70%. SAVE existente tem prioridade.</p><p id="chordStatus" role="status">Banco global de acordes · salvo automaticamente neste navegador.</p><div id="chordBank" class="chord-bank"></div></section><section class="instrument"''')
html=html.replace('0.3.0-sc1','0.3.0-ap8').replace('SC1 anterior · referência preservada','AP8 oficial · referência preservada').replace('Restaurar SC1','Restaurar AP8').replace('0.3.0-ap8</h2><p id="buildNoteA">AP8 oficial · preset inicial de Marcus','0.3.0-ch4</h2><p id="buildNoteA">CH4 · Pad Chords').replace('Restaurar AP8</button></article><article','Restaurar CH3</button></article><article')
html=html.replace('href="install-appearance/"','href="../install-appearance/"').replace('href="downloads/','href="../downloads/').replace('href="lab-builds/','href="../lab-builds/').replace('<b>B</b><span>SC1','<b>B</b><span>AP8').replace('<b>A</b><span>AP8','<b>A</b><span>CH1')
html=html.replace('1–4 timbres · 5–8 layers · 9–16 funções','Modo acordes: 1–8 acordes · 9–16 controles').replace('A · AP8 &nbsp; / &nbsp; B · SC1','A · CH1 &nbsp; / &nbsp; B · AP8')
html=html.replace('<p><b>SC1:</b>', '<p><b>SC1:</b>')
html=html.replace('href="../downloads/SMKTRON-Appearance-fontes.zip"','href="source.zip"').replace('Fontes AP8 e licenças','Fontes CH3 e licenças')
html=html.replace('href="../install-appearance/"','href="https://smktron-rf1.padrini.chatgpt.site/install/"').replace('href="../lab-builds/CREDITOS-SC1.md"','href="https://smktron-simulator.padrini.chatgpt.site/lab-builds/CREDITOS-SC1.md"').replace('href="../"','href="https://smktron-simulator.padrini.chatgpt.site/"')
html=html.replace('<span>CH1</span>','<span>CH3</span>').replace('A · CH1','A · CH3')
html=html.replace('<main>','<main><p class="build-hint"><a href="../manual/">Manual do SMKTRON · telas e funções</a></p>',1)
(O/'index.html').write_text(html)
with (O/'style.css').open('a') as f:f.write('\n.chord-help{border:1px solid #b3804e;background:#211e19;border-radius:12px;padding:18px;margin:18px 0;color:#eadbc8}.chord-help>div:first-child{display:flex;align-items:center;justify-content:space-between;gap:12px;flex-wrap:wrap}.chord-help p{font-size:13px;line-height:1.6;max-width:1000px}.chord-bank{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}.chord-bank span{padding:8px;background:#111b19;border:1px solid #47665a;border-radius:6px;font-size:12px}.chord-bank .active{border-color:#ff9a35;color:#ffbc70}#chordMode[aria-pressed=true]{background:#c38445;color:#111}@media(max-width:600px){.chord-bank{grid-template-columns:repeat(2,1fr)}}\n')
app=(D/'app.js').read_text().replace('v=ap8','v=ch4').replace("'SYSTEM'];","'SYSTEM','PAD CHORDS'];")
a=app.index('const defaultBuilds=');b=app.index('\nlet builds=',a)
app=app[:a]+"const defaultBuilds={A:{name:'0.3.0-ch4',url:'mel.wasm?v=ch4',note:'CH4 oficial · Pad Chords + HOLD'},B:{name:'0.3.0-ap8',url:'../appearance-preview/mel.wasm',note:'AP8 oficial · referência preservada'}};"+app[b:]
app=app.replace("['PAD LIGHT','THEME','MASTER','—']];","['PAD LIGHT','THEME','MASTER','—'],['MIX','PAN MELODY','PAN CHORDS','MODE']];")
app=app.replace('function parameterValue(pg,i,v){','function parameterValue(pg,i,v){\n if(pg===13)return i===0?v+\'%\':i===3?(v?\'ON\':\'OFF\'):v===0?\'CENTER\':(v<0?\'L\':\'R\')+Math.abs(v);')
app=app.replace("$('#panic').onclick=panic;","$('#panic').onclick=panic;\n$('#chordMode').onclick=()=>send({type:'chord-mode',on:!$('#chordMode').matches('[aria-pressed=true]')});")
app=app.replace(" for(const s of m.stores||[])",""" if(m.chords){
  const {state,bank}=m.chords,[on,hold,rec,target,playing,tape,,mix,panMel,panChords]=state;
  $('#chordMode').disabled=false;$('#chordMode').setAttribute('aria-pressed',!!on);$('#chordMode').textContent=on?'Sair de Pad Chords':'Ativar Pad Chords';
  $('#chordStatus').textContent=`${on?'ATIVO':'DESATIVADO'} · ${SOUNDS[tape]} · MIX ${mix}% · PAN MEL ${panMel===0?'CENTER':(panMel<0?'L':'R')+Math.abs(panMel)} · PAN CHD ${panChords===0?'CENTER':(panChords<0?'L':'R')+Math.abs(panChords)} · HOLD ${hold?'ON':'OFF'} · ${rec?(target<8?'Gravando pad '+(target+1)+' — toque e solte todas as teclas':'REC — selecione pad 1–8'):'Banco global salvo automaticamente'}`;
  $('#chordBank').replaceChildren(...bank.map((notes,i)=>{const x=document.createElement('span');x.classList.toggle('active',playing===i);x.textContent=`${i+1} · `+(notes.length?notes.map(n=>noteNames[n%12]+(Math.floor(n/12)-1)).join(' '):'vazio');return x;}));
  if(on){const labels=[...bank.map((ns,i)=>rec&&target===i?'REC · TOQUE ACORDE':ns.length?ns.map(n=>noteNames[n%12]+(Math.floor(n/12)-1)).join(' '):'VAZIO'),'REC','HOLD',...SOUNDS,'STOP / CANCEL','SAIR'];$$('.pad').forEach((b,i)=>{b.disabled=false;b.querySelector('small').textContent=labels[i];b.setAttribute('aria-label',`Pad ${i+1} ${labels[i]}`);b.setAttribute('aria-pressed',i<8?i===playing:i===8?!!rec:i===9?!!hold:i>=10&&i<=13?i===tape+10:false);});}
 }else{$('#chordMode').disabled=true;$('#chordStatus').textContent='Selecione a build CH3 em A para testar o modo acordes.';$('#chordBank').replaceChildren();}
 for(const s of m.stores||[])""")
app=app.replace("if(s.sceneStore)localStorage.setItem", "if(s.chordStore)localStorage.setItem(storageKey('ch1','global-chords'),JSON.stringify(s.chordStore));if(s.sceneStore)localStorage.setItem")
app=app.replace("sceneStore:clean?null:stored(builds[id].hash,'scenes')", "sceneStore:clean?null:stored(builds[id].hash,'scenes'),chordStore:stored('ch1','global-chords')")
app=app.replace("const req=indexedDB.open('smktron-lab-builds',1)","const req=indexedDB.open('smktron-chords-lab-builds',1)")
app=app.replace("b=await cache(id);if(b)b=await verify","b=await cache(id);if(['0.3.0-ch1','0.3.0-ch2','0.3.0-ch3'].includes(b?.name))b=null;if(b)b=await verify")
app=app.replace('const [on,pin,mask,selected]=m.scenes;', 'const [on,pin,mask,selected,lastLoaded=selected]=m.scenes;').replace('i===selected:i===8','i===lastLoaded:i===8')
(O/'app.js').write_text(app)
shutil.copy2(D/'worklet.js',O/'worklet.js')
p=O/'worklet.js';p.write_text(p.read_text().replace('v=ap8','v=ch4'))
lab=(D/'lab-engine.js').read_text()
lab=lab.replace('   e.web_master?.(700);e.web_boot();',"   if(e.web_chord_flash&&bytesOK(b.chordStore,8192)&&b.chordStore.length===8192)new Uint8Array(e.memory.buffer,e.web_chord_flash(),8192).set(b.chordStore);\n   e.web_master?.(700);e.web_boot();")
lab=lab.replace('sceneWrites:e.web_scene_writes?.()||0','sceneWrites:e.web_scene_writes?.()||0,chordWrites:e.web_chord_writes?.()||0')
lab=lab.replace("if((m.type==='key'&&m.vel>0)","if((m.type==='pad'&&m.vel>0)||(m.type==='key'&&m.vel>0)")
lab=lab.replace("for(const {e} of Object.values(this.slots))switch(m.type){", "for(const {e} of Object.values(this.slots)){if(['pad','pad-cancel'].includes(m.type)&&this.slots[this.active].e.web_chord_state?.(0)&&e!==this.slots[this.active].e)continue;switch(m.type){\n   case 'chord-mixer':e.web_chord_mixer?.(m.k,m.value);break;\n   case 'chord-mode':e.web_chord_mode?.(m.on?1:0);this.muted=false;break;")
lab=lab.replace("case 'pad-cancel':e.web_scene_cancel_pad?", "case 'pad-cancel':e.web_chord_state?.(0)?e.web_pad(m.k,0):e.web_scene_cancel_pad?")
lab=lab.replace("e.web_probe(0)===12||this.slots[this.active].e.web_probe(0)===12", "e.web_probe(0)>=12||this.slots[this.active].e.web_probe(0)>=12")
lab=lab.replace("  }\n }\n stop()", "  }}\n }\n stop()")
lab=lab.replace("  for(const [id,s] of Object.entries(this.slots)){\n   const x=", "  if(e.web_chord_state)msg.chords={state:Array.from({length:10},(_,i)=>e.web_chord_state(i)),bank:Array.from({length:8},(_,p)=>Array.from({length:6},(_,i)=>e.web_chord_note(p,i)).filter(n=>n>=0))};\n  for(const [id,s] of Object.entries(this.slots)){\n   const x=")
lab=lab.replace("   if(changed)msg.stores.push(save);", "   if(x.web_chord_writes&&s.chordWrites!==x.web_chord_writes()){s.chordWrites=x.web_chord_writes();save.chordStore=Array.from(new Uint8Array(x.memory.buffer,x.web_chord_flash(),8192));changed=true;}\n   if(changed)msg.stores.push(save);")
lab=lab.replace('e.web_scene_selected()];','e.web_scene_selected(),e.web_scene_loaded?.()??e.web_scene_selected()];')
(O/'lab-engine.js').write_text(lab)
