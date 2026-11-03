// SPDX-License-Identifier: GPL-3.0-only
const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
const STORE='smktron-grain-gr3-boot-v1',SCSTORE='smktron-grain-gr3-scenes-v1',CHSTORE='smktron-grain-gr3-chords-v1';
try{if(!localStorage.getItem('smktron-gr3-imported')){for(const [to,from] of [[STORE,'smktron-scenes-sc1-boot-v1'],[SCSTORE,'smktron-scenes-sc1-slots-v1'],[CHSTORE,'smktron-pad-chords-global-v1']]){const value=localStorage.getItem(from);if(value&&!localStorage.getItem(to))localStorage.setItem(to,value)}localStorage.setItem('smktron-gr3-imported','1')}}catch{}
const PAGES=['PLAY','LAYER','TAPE','WEAR','VIBRATO','LESLIE','ECHO','REVERB','EQ','SETUP','GRAIN','SEQ','SYSTEM','PAD CHORDS'];
const SOUNDS=['FLUTE','CHOIR','CELLO','VIOLINS'];
let ctx,node,ready=false,starting=false,midiAccess,midiPort,lastState=[];
const sources=new Map(), pointerNotes=new Map(),heldButtons=new Set(),heldPads=new Set();
function status(s){$('#status').textContent=s}
function send(m){if(ready&&node){node.port.postMessage(m);return true}if(!starting)status('Clique em Iniciar áudio para tocar.');return false}
function key(k,source,down){
 const held=sources.get(k)||new Set(),was=held.size;
 if(down)held.add(source);else held.delete(source);
 if(held.size)sources.set(k,held);else sources.delete(k);
 if(!!was!==!!held.size)send({type:'key',k,vel:held.size?127:0});
 $(`[data-key="${k}"]`)?.classList.toggle('down',held.size>0);
}
function releaseAll(){for(const [k,held] of [...sources])for(const src of [...held])key(k,src,false);pointerNotes.clear();for(const i of heldPads)send({type:'pad-cancel',k:i});heldPads.clear();for(const id of heldButtons)send(id===11?{type:'save-cancel'}:{type:'button',id,down:false});heldButtons.clear();$$('[data-btn]').forEach(b=>b.classList.remove('down'))}
function panic(){releaseAll();if(ready)send({type:'panic'});$('#sustain').checked=false;status('Notas liberadas · áudio ativo')}
const defs=[[12,'PATCH','PLAY'],[8,'SCALE','LAYER'],[7,'NOTE REPEAT','LOFI'],[14,'FX','EFEITOS'],[13,'PARA','EQ'],[10,'GLOBE','SETUP'],[11,'BT','SALVAR'],[6,'GRAIN','FRAGMENTOS'],[15,'SEQ','SEQUÊNCIA'],[16,'SEQ PLAY','TOCAR'],[17,'SEQ REC','GRAVAR'],[9,'CHORD','HOLD'],[4,'OCT −','OITAVA'],[5,'OCT +','OITAVA'],[0,'KNOB BANK','PAD MODES']];
for(const [id,label,sub] of defs){let b=document.createElement('button');b.dataset.btn=id;b.innerHTML=`${label}<small>${sub}</small>`;$('#buttons').append(b)}
for(const b of $$('[data-btn]')){
 const id=+b.dataset.btn;
 function down(){if(!ready){status('Inicie o áudio para usar o painel.');return}heldButtons.add(id);b.classList.add('down');send({type:'button',id,down:true})}
 function up(){if(!heldButtons.has(id))return;heldButtons.delete(id);b.classList.remove('down');send({type:'button',id,down:false})}
 function cancel(){if(!heldButtons.delete(id))return;b.classList.remove('down');send(id===11?{type:'save-cancel'}:{type:'button',id,down:false})}
 b.addEventListener('pointerdown',e=>{e.preventDefault();b.setPointerCapture(e.pointerId);down()});b.addEventListener('pointerup',up);b.addEventListener('pointercancel',cancel);b.addEventListener('lostpointercapture',cancel);
 b.addEventListener('click',e=>{if(e.detail===0){down();setTimeout(up,70)}});
}
const knobDefs=[[0,'K1 · PARÂMETRO 1'],[1,'K2 · PARÂMETRO 2'],[2,'K3 · PARÂMETRO 3'],[3,'K4 · PARÂMETRO 4'],[5,'K6 · PÁGINAS'],[6,'K7 · TEMPO'],[7,'K8 · TIMBRE']];
for(const [id,label] of knobDefs){
 let w=document.createElement('div');w.className='knob-wrap';w.dataset.knob=id;w.innerHTML=`<div class="knob" role="slider" aria-label="${label}" aria-valuemin="-1000" aria-valuemax="1000" aria-valuenow="0" tabindex="0"></div><label>${label}</label><div class="nudge"><button aria-label="Diminuir ${label}">−</button><button aria-label="Aumentar ${label}">+</button></div>`;$('#knobs').append(w);
 let val=0,y;const knob=w.querySelector('.knob');function turn(n){if(send({type:'enc',id,n})){val+=n;knob.style.setProperty('--angle',`${val*8-30}deg`);knob.setAttribute('aria-valuenow',val)}}
 w.querySelectorAll('button').forEach((b,i)=>b.onclick=()=>turn(i?1:-1));
 knob.onpointerdown=e=>{y=e.clientY;knob.setPointerCapture(e.pointerId)};knob.onpointermove=e=>{if(y===undefined)return;let d=Math.trunc((y-e.clientY)/5);if(d){turn(d);y-=d*5}};knob.onpointerup=knob.onpointercancel=()=>{y=undefined};
 knob.onwheel=e=>{e.preventDefault();turn(e.deltaY<0?1:-1)};knob.onkeydown=e=>{if(['ArrowUp','ArrowRight','ArrowDown','ArrowLeft'].includes(e.key)){e.preventDefault();turn(['ArrowUp','ArrowRight'].includes(e.key)?1:-1)}};
}
const padNames=['FLUTE','CHOIR','CELLO','VIOLINS','LYR FLUTE','LYR CHOIR','LYR CELLO','LYR VIOLINS','LOFI','GRAIN','LESLIE','FAST/SLOW','ECHO','REVERB','HALF','8s / HELD'];
for(let i=0;i<16;i++){
 let b=document.createElement('button');b.className='pad';b.innerHTML=`<strong>${String(i+1).padStart(2,'0')}</strong><small>${padNames[i]}</small>`;b.setAttribute('aria-label',`Pad ${i+1} ${padNames[i]}`);$('#pads').append(b);
 b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);heldPads.add(i);send({type:'pad',k:i,vel:100})};b.onpointerup=()=>{if(heldPads.delete(i))send({type:'pad',k:i,vel:0})};b.onpointercancel=b.onlostpointercapture=()=>{if(heldPads.delete(i))send({type:'pad-cancel',k:i})};b.onclick=e=>{if(e.detail===0){send({type:'pad',k:i,vel:100});setTimeout(()=>send({type:'pad',k:i,vel:0}),70)}};
}
for(let i=0;i<4;i++){
 let d=document.createElement('label');d.className='fader';d.innerHTML=`<input aria-label="Fader ${i+1}" type="range" min="0" max="127" value="0"><small>F${i+1}</small>`;$('#faders').append(d);d.querySelector('input').oninput=e=>send({type:'fader',id:i,value:+e.target.value});
}
let white=0;const whiteCount=22,blacks=[1,3,6,8,10];const noteNames=['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'];
for(let k=0;k<37;k++){
 const black=blacks.includes(k%12),b=document.createElement('button');b.className=`key ${black?'black':'white'}`;b.dataset.key=k;
 const pos=black?white/whiteCount*100-1.3:white/whiteCount*100;b.style.left=pos+'%';b.style.width=(black?2.6:100/whiteCount)+'%';if(!black)white++;
 const name=noteNames[k%12]+(3+Math.floor(k/12));b.setAttribute('aria-label',name);b.innerHTML=`<span>${k%12===0?name:''}</span>`;$('#keyboard').append(b);
 b.addEventListener('pointerdown',e=>{e.preventDefault();b.setPointerCapture(e.pointerId);pointerNotes.set(e.pointerId,k);key(k,'p'+e.pointerId,true)});
 b.addEventListener('pointermove',e=>{if(!pointerNotes.has(e.pointerId))return;const target=document.elementFromPoint(e.clientX,e.clientY)?.closest('[data-key]');if(!target)return;let next=+target.dataset.key,prev=pointerNotes.get(e.pointerId);if(next!==prev){key(prev,'p'+e.pointerId,false);key(next,'p'+e.pointerId,true);pointerNotes.set(e.pointerId,next)}});
 const up=e=>{const prev=pointerNotes.get(e.pointerId);if(prev!==undefined)key(prev,'p'+e.pointerId,false);pointerNotes.delete(e.pointerId)};b.addEventListener('pointerup',up);b.addEventListener('pointercancel',up);b.addEventListener('lostpointercapture',up);
 b.addEventListener('click',e=>{if(e.detail===0){key(k,'accessible',true);setTimeout(()=>key(k,'accessible',false),180)}});
}
const KEYMAP={KeyA:12,KeyW:13,KeyS:14,KeyE:15,KeyD:16,KeyF:17,KeyT:18,KeyG:19,KeyY:20,KeyH:21,KeyU:22,KeyJ:23,KeyK:24,KeyO:25,KeyL:26,KeyP:27,Semicolon:28};
window.addEventListener('keydown',e=>{if(e.target?.matches?.('input,select,textarea')||e.ctrlKey||e.metaKey||e.altKey)return;if(e.code==='KeyB'){e.preventDefault();if(!e.repeat){heldButtons.add(11);$('[data-btn="11"]').classList.add('down');send({type:'button',id:11,down:true})}return}if(e.code in KEYMAP){e.preventDefault();if(!e.repeat)key(KEYMAP[e.code],e.code,true)}});
window.addEventListener('keyup',e=>{if(e.code==='KeyB'){e.preventDefault();if(heldButtons.delete(11)){send({type:'button',id:11,down:false});$('[data-btn="11"]').classList.remove('down')}return}if(e.code in KEYMAP){e.preventDefault();key(KEYMAP[e.code],e.code,false)}});
window.addEventListener('blur',()=>{releaseAll();if(ready){send({type:'midi',data:[0xB0,64,0]});$('#sustain').checked=false}});
$('#volume').oninput=e=>{$('#volumeVal').textContent=Math.round(+e.target.value/1023*100)+'%';send({type:'master',value:+e.target.value})};
$('#pitch').oninput=e=>send({type:'wheel',id:0,value:+e.target.value});const resetPitch=()=>{$('#pitch').value=535;send({type:'wheel',id:0,value:535})};$('#pitch').onpointerup=$('#pitch').onpointercancel=resetPitch;$('#pitch').onkeyup=resetPitch;
$('#mod').oninput=e=>send({type:'wheel',id:1,value:+e.target.value});
$('#sustain').onchange=e=>send({type:'midi',data:[0xB0,64,e.target.checked?127:0]});
$('#panic').onclick=panic;
const canvas=$('#screen'),g=canvas.getContext('2d'),pixels=g.createImageData(240,240);
const PARAMS=[["TONE", "PITCH", "ATTACK", "RELEASE"], ["SOUND", "MIX", "OCTAVE", "DETUNE"], ["WOW", "FLUTTER", "DRIVE", "HISS"], ["AGE", "DROPOUT", "DRIFT", "CRUSH"], ["RATE", "DEPTH", "TONE", "MAX MIX"], ["SPEED", "DRIVE", "SPREAD", "MIX"], ["TIME", "REPEATS", "TONE", "MIX"], ["SIZE", "TONE", "PRE", "MIX"], ["LOW", "MID", "FREQ", "HIGH"], ["TEMPO", "TUNE", "LENGTH", "SCREEN"], ["SIZE", "SPREAD", "DENSITY", "MIX"], ["LENGTH", "GATE", "STEP", "SWING"], ["PAD LIGHT", "THEME", "MASTER", "\u2014"],["MIX","PAN MELODY","PAN CHORDS","MODE"]];
function onMessage(m){
 if(m.type==='error'){status('Erro no áudio: '+m.message);$('#start').textContent='Tentar novamente';$('#start').disabled=false;ready=false;starting=false;return}
 if(m.type==='ready'){ready=true;$('#volume').value=m.master;$('#volumeVal').textContent=Math.round(m.master/1023*100)+'%';starting=false;$('#start').disabled=false;$('#start').textContent='Ⅱ Pausar áudio';$('#screenCover').style.display='none';status('Áudio ativo · 44,1 kHz');return}
 if(m.type!=='frame')return;
 if(m.fb){for(let i=0;i<m.fb.length;i++){const p=m.fb[i],j=i*4;pixels.data[j]=((p>>11)&31)*255/31;pixels.data[j+1]=((p>>5)&63)*255/63;pixels.data[j+2]=(p&31)*255/31;pixels.data[j+3]=255}g.putImageData(pixels,0,0)}
 if(m.state){lastState=m.state;$('#volume').value=m.state[17];$('#volumeVal').textContent=Math.round(m.state[17]/1023*100)+'%';$('#screenState').textContent=(PAGES[m.state[0]]||'PLAY')+' · '+(SOUNDS[m.state[1]]||'FLUTE');for(const [id,on] of [[6,m.grain?.[0]||0],[16,m.state[6]],[17,m.state[7]]]){$(`[data-btn="${id}"]`)?.setAttribute('aria-pressed',!!on)}}
 if(m.state){const pg=m.state[0];const k6=document.querySelector('[data-knob="5"]');k6.querySelector('label').textContent=pg===10?'K6 · STYLE':'K6 · PÁGINAS';k6.querySelector('.knob').setAttribute('aria-label',pg===10?'K6 · STYLE':'K6 · PÁGINAS');for(let i=0;i<4;i++){const w=document.querySelector('[data-knob="'+i+'"]'),name='K'+(i+1)+' · '+PARAMS[pg][i];w.querySelector('label').textContent=name;w.querySelector('.knob').setAttribute('aria-label',name);}}
 if(m.bank!==undefined){$('#faderBank').textContent='Banco '+(m.bank?'B':'A');$('#faderBank').setAttribute('aria-pressed',!!m.bank);const names=m.bank?['AGE','DRIVE','DELAY','PLATE']:['LYR MIX','TONE','WOW','FLUTTER'];$$('.fader').forEach((f,i)=>{f.querySelector('small').textContent='F'+(i+1)+' · '+names[i];f.querySelector('input').setAttribute('aria-label','Fader '+(i+1)+' '+names[i]);});}
 if(m.pads){$$('.pad').forEach((b,i)=>{const c=m.pads[i],r=(c>>8)&255,green=(c>>16)&255,blue=c&255;b.style.background=`rgb(${r},${green},${blue})`;b.style.color=green>90?'#142725':'#f1d2ad'})}
 if(m.settings){try{localStorage.setItem(STORE,JSON.stringify(m.settings))}catch{}}
 if(m.chordStore){try{localStorage.setItem(CHSTORE,JSON.stringify(m.chordStore))}catch{status('Não foi possível salvar os acordes neste navegador.')}}
 if(m.sceneStore){try{localStorage.setItem(SCSTORE,JSON.stringify(m.sceneStore))}catch{status('As cenas funcionam nesta sessão, mas o navegador não permitiu salvá-las.')}}
 if(m.scenes)$('#padModeLabel').textContent=m.chords?.state[0]?'CHORD PADS':m.scenes[0]?'PRESETS':'PERFORMANCE';
 if(m.scenes){const [on,pin,mask,selected,lastLoaded=selected]=m.scenes;$$('.pad').forEach((b,i)=>{const label=!on?padNames[i]:i<8?`S${String(i+1).padStart(2,'0')} · ${mask&(1<<i)?'GRAVADA':'VAZIA'}`:i===8?(pin?'FIXADO · SAIR':'FIXAR PRESETS'):'';b.querySelector('small').textContent=label;b.disabled=!!on&&i>8;b.setAttribute('aria-label',`Pad ${i+1} ${label}`);b.setAttribute('aria-pressed',!!on&&(i<8?i===lastLoaded:i===8&&!!pin))});if(on)$('#screenState').textContent+=(pin?' · PRESETS':' · SCENES')+(selected<8?' S'+String(selected+1).padStart(2,'0'):'')}
 if(m.chords&&m.chords.state[0]){const [on,hold,rec,target,playing,tape]=m.chords.state;const labels=[...m.chords.bank.map((notes,i)=>rec&&target===i?'REC · TOQUE ACORDE':notes.length?notes.map(n=>noteNames[n%12]+(Math.floor(n/12)-1)).join(' '):'VAZIO'),'REC','HOLD',...SOUNDS,'STOP / CANCEL','SAIR'];$$('.pad').forEach((b,i)=>{b.disabled=false;b.querySelector('small').textContent=labels[i];b.setAttribute('aria-label',`Pad ${i+1} ${labels[i]}`);b.setAttribute('aria-pressed',i<8?i===playing:i===8?!!rec:i===9?!!hold:i>=10&&i<=13?i===tape+10:false);});}

}
$('#start').onclick=async()=>{
 if(starting)return;
 if(ready&&ctx){if(ctx.state==='running'){releaseAll();send({type:'panic'});await ctx.suspend();$('#start').textContent='▶ Retomar áudio';status('Áudio pausado')}else{await ctx.resume();$('#start').textContent='Ⅱ Pausar áudio';status('Áudio ativo · 44,1 kHz')}return}
 starting=true;$('#start').disabled=true;status('Carregando instrumento…');
 try{
  if(ctx)await ctx.close();ctx=new AudioContext({sampleRate:44100,latencyHint:'interactive'});await ctx.resume();
  if(ctx.sampleRate!==44100)throw Error('Este navegador não disponibilizou áudio em 44,1 kHz. Tente Chrome ou Edge.');
  if(!ctx.audioWorklet)throw Error('Áudio indisponível neste contexto. Abra o simulador pelo endereço HTTPS em um navegador compatível.');await ctx.audioWorklet.addModule('worklet.js?v=grain-gr3');const response=await fetch('mel.wasm?v=grain-gr3');if(!response.ok)throw Error('Não foi possível carregar o motor de áudio.');const wasm=await response.arrayBuffer();
  node=new AudioWorkletNode(ctx,'smktron',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2]});node.connect(ctx.destination);node.port.onmessage=e=>onMessage(e.data);
  let settings;try{settings=JSON.parse(localStorage.getItem(STORE))}catch{}
  if(!Array.isArray(settings)||settings.some(n=>!Number.isInteger(n)||n<0||n>255))settings=null;
  let sceneStore;try{sceneStore=JSON.parse(localStorage.getItem(SCSTORE))}catch{}
  if(!Array.isArray(sceneStore)||sceneStore.length!==8192||sceneStore.some(n=>!Number.isInteger(n)||n<0||n>255))sceneStore=null;
  let chordStore;try{chordStore=JSON.parse(localStorage.getItem(CHSTORE))}catch{}
  if(!Array.isArray(chordStore)||chordStore.length!==8192||chordStore.some(n=>!Number.isInteger(n)||n<0||n>255))chordStore=null;
  node.port.postMessage({type:'load',wasm,settings,sceneStore,chordStore});
 }catch(e){status(e.message);$('#start').disabled=false;starting=false;ready=false;if(ctx)await ctx.close();ctx=null}
};
let midiGeneration=0;
async function attachMidi(){
 const generation=++midiGeneration;
 const previous=midiPort;if(previous){previous.onmidimessage=null;await previous.close().catch(()=>{});}
 if(ready)send({type:'panic'});
 if(generation!==midiGeneration)return;
 midiPort=midiAccess?.inputs.get($('#midiInput').value)||null;
 const port=midiPort;
 if(!port){$('#midiStatus').textContent='Conecte um teclado MIDI.';return;}
 try{await port.open();if(generation!==midiGeneration)return;
 port.onmidimessage=e=>{if(ready&&ctx?.state==='running')send({type:'midi',data:Array.from(e.data)})};
 $('#midiStatus').textContent='MIDI: '+(port.name||'Entrada MIDI');
 }catch(e){$('#midiStatus').textContent='MIDI: '+e.message;}
}
function midiList(){
 const old=$('#midiInput').value;
 $('#midiInput').replaceChildren();
 for(const p of midiAccess.inputs.values()){if(p.state==='disconnected')continue;const o=document.createElement('option');o.value=p.id;o.textContent=p.name||'Entrada MIDI';$('#midiInput').append(o)}
 $('#midiInput').hidden=!$('#midiInput').options.length;
 if(old&&midiAccess.inputs.has(old))$('#midiInput').value=old;
 if(!$('#midiInput').value&&$('#midiInput').options.length)$('#midiInput').selectedIndex=0;
 attachMidi();
}
$('#midi').onclick=async()=>{try{
 if(!navigator.requestMIDIAccess)throw Error('MIDI não disponível neste navegador. Use Chrome ou Edge.');
 if(!ready)await $('#start').onclick();
 midiAccess??=await navigator.requestMIDIAccess({sysex:false});midiAccess.onstatechange=midiList;midiList();
}catch(e){$('#midiStatus').textContent='MIDI: '+e.message}};
$('#midiInput').onchange=()=>attachMidi();
$('#reset').onclick=()=>{if(confirm(window.smktronText('Restaurar os ajustes e apagar as oito cenas e os acordes deste simulador? Isso não altera o teclado físico.'))){localStorage.removeItem(STORE);localStorage.removeItem(SCSTORE);localStorage.removeItem(CHSTORE);location.reload()}};

