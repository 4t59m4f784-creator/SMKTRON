import {validateModule} from './lab-engine.js?v=ap8';
// SPDX-License-Identifier: GPL-3.0-only
const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
const storageKey=(hash,kind)=>'smktron-lab-'+hash+'-'+kind;
const PAGES=['PLAY','LAYER','TAPE','WEAR','VIBRATO','LESLIE','ECHO','REVERB','EQ','SETUP','ARP','SEQ','SYSTEM'];
const SOUNDS=['FLUTE','CHOIR','CELLO','VIOLINS'];
let ctx,node,ready=false,starting=false,midiAccess,midiPort,lastState=[],active='A',phrasePlaying=false,audioTimeout;
const defaultBuilds={A:{name:'0.3.0-ap8',url:'appearance-preview/mel.wasm',note:'AP8 oficial · preset inicial de Marcus'},B:{name:'0.3.0-sc1',url:'lab-builds/sc1.wasm',note:'SC1 anterior · referência preservada'}};
let builds={},prepared;
const sources=new Map(), pointerNotes=new Map(),heldButtons=new Set(),heldPads=new Set();
function status(s){$('#status').textContent=s}
function send(m){if(ready&&node){node.port.postMessage(m);return true}if(!starting)status('Clique em Iniciar áudio para tocar.');return false}
function key(k,source,down){
 const held=sources.get(k)||new Set(),was=held.size;
 if(down&&!ready){status('Inicie o áudio para tocar.');return}if(down)held.add(source);else held.delete(source);
 if(held.size)sources.set(k,held);else sources.delete(k);
 if(!!was!==!!held.size)send({type:'key',k,vel:held.size?+$('#velocity').value:0});
 $(`[data-key="${k}"]`)?.classList.toggle('down',held.size>0);
}
function releaseAll(){for(const [k,held] of [...sources])for(const src of [...held])key(k,src,false);pointerNotes.clear();for(const i of heldPads)send({type:'pad-cancel',k:i});heldPads.clear();for(const id of heldButtons)send(id===11?{type:'save-cancel'}:{type:'button',id,down:false});heldButtons.clear();$$('[data-btn]').forEach(b=>b.classList.remove('down'))}
function panic(){releaseAll();if(ready)send({type:'panic'});$('#sustain').checked=false;phrasePlaying=false;$('#phrase').textContent='▷ Frase de teste';status(ready?'Notas e transportes liberados':'Pronto para iniciar')}
const defs=[[12,'PATCH','PLAY'],[8,'SCALE','LAYER'],[7,'NOTE REPEAT','LOFI'],[14,'FX','EFEITOS'],[13,'PARA','EQ'],[10,'GLOBE','SETUP'],[11,'SAVE','BT'],[6,'ARP','ON / OFF'],[15,'SEQ','SEQUÊNCIA'],[16,'SEQ PLAY','TOCAR'],[17,'SEQ REC','GRAVAR'],[9,'CHORD','HOLD'],[4,'OCT −','OITAVA'],[5,'OCT +','OITAVA'],[0,'DIAG','KNOB BANK']];
for(const [id,label,sub] of defs){let b=document.createElement('button');b.dataset.btn=id;b.innerHTML=`${label}<small>${sub}</small>`;b.title=label+' · '+sub;$('#buttons').append(b)}
for(const [i,name] of PAGES.entries()){const b=document.createElement('button');b.textContent=name==='ECHO'?'DELAY':name==='REVERB'?'PLATE':name;b.dataset.page=i;b.setAttribute('aria-pressed',i===0);b.onclick=()=>send({type:'page',page:i});$('#pages').append(b)}
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
 let w=document.createElement('div');w.className='knob-wrap';w.innerHTML=`<div class="knob" role="slider" aria-label="${label}" aria-valuemin="-1000" aria-valuemax="1000" aria-valuenow="0" tabindex="0"></div><label>${label}</label><output>—</output><div class="nudge"><button aria-label="Diminuir ${label}">−</button><button aria-label="Aumentar ${label}">+</button></div>`;$('#knobs').append(w);
 let val=0,y;const knob=w.querySelector('.knob');function turn(n){if(send({type:'enc',id,n})){val=Math.max(-1000,Math.min(1000,val+n));knob.style.setProperty('--angle',`${val*8-30}deg`);knob.setAttribute('aria-valuenow',val)}}
 w.querySelectorAll('button').forEach((b,i)=>b.onclick=()=>turn(i?1:-1));
 knob.onpointerdown=e=>{y=e.clientY;knob.setPointerCapture(e.pointerId)};knob.onpointermove=e=>{if(y===undefined)return;let d=Math.trunc((y-e.clientY)/5);if(d){turn(d);y-=d*5}};knob.onpointerup=knob.onpointercancel=()=>{y=undefined};
 knob.onwheel=e=>{e.preventDefault();turn(e.deltaY<0?1:-1)};knob.onkeydown=e=>{if(['ArrowUp','ArrowRight','ArrowDown','ArrowLeft'].includes(e.key)){e.preventDefault();turn(['ArrowUp','ArrowRight'].includes(e.key)?1:-1)}};
}
const padNames=['FLUTE','CHOIR','CELLO','VIOLINS','+ FLUTE','+ CHOIR','+ CELLO','+ VIOLINS','LOFI','VIBRATO','LESLIE','FAST / SLOW','DELAY','PLATE','HALF','8s / HELD'];
for(let i=0;i<16;i++){
 let b=document.createElement('button');b.className='pad';b.dataset.pad=i;b.innerHTML=`<strong>${String(i+1).padStart(2,'0')}</strong><small>${padNames[i]}</small>`;b.setAttribute('aria-label',`Pad ${i+1} ${padNames[i]}`);$('#pads').append(b);
 b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);if(!ready){status('Inicie o áudio para usar os pads.');return}heldPads.add(i);send({type:'pad',k:i,vel:100})};b.onpointerup=()=>{if(heldPads.delete(i))send({type:'pad',k:i,vel:0})};b.onpointercancel=b.onlostpointercapture=()=>{if(heldPads.delete(i))send({type:'pad-cancel',k:i})};b.onclick=e=>{if(e.detail===0){send({type:'pad',k:i,vel:100});setTimeout(()=>send({type:'pad',k:i,vel:0}),70)}};
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
window.addEventListener('blur',()=>{releaseAll();if(ready){send({type:'panic'});$('#sustain').checked=false;resetPitch()}});
document.addEventListener('visibilitychange',()=>{if(document.hidden)panic()});
$('#velocity').oninput=e=>{$('#velVal').value=e.target.value};
$('#volume').oninput=e=>{$('#volumeVal').textContent=Math.round(+e.target.value/1023*100)+'%';send({type:'master',value:+e.target.value})};
$('#pitch').oninput=e=>send({type:'wheel',id:0,value:+e.target.value});const resetPitch=()=>{$('#pitch').value=535;send({type:'wheel',id:0,value:535})};$('#pitch').onpointerup=$('#pitch').onpointercancel=resetPitch;$('#pitch').onkeyup=resetPitch;
$('#mod').oninput=e=>send({type:'wheel',id:1,value:+e.target.value});
$('#sustain').onchange=e=>send({type:'midi',data:[0xB0,64,e.target.checked?127:0]});
$('#panic').onclick=panic;
const canvas=$('#screen'),g=canvas.getContext('2d'),pixels=g.createImageData(240,240);
const PARAMS=[['TONE','PITCH','ATTACK','RELEASE'],['SOUND','MIX','OCTAVE','DETUNE'],['WOW','FLUTTER','DRIVE','HISS'],['AGE','DROPOUT','DRIFT','CRUSH'],['RATE','DEPTH','TONE','MIX'],['SPEED','DRIVE','SPREAD','MIX'],['TIME','REPEATS','TONE','MIX'],['SIZE','TONE','PRE','MIX'],['LOW','MID','FREQ','HIGH'],['TEMPO','TUNE','LENGTH','SCREEN'],['MODE','RATE','OCTAVES','GATE'],['LENGTH','GATE','STEP','SWING'],['PAD LIGHT','THEME','MASTER','—']];
function parameterValue(pg,i,v){
 if(pg===12)return (i===0||i===2&&v>=0)?v+'%':i===1?(v?'DARK':'LIGHT'):'—';
 if(pg===1&&i===0)return ['OFF',...SOUNDS][v]||v;
 if(pg===5&&i===0)return ['STOP','SLOW','FAST'][v]||v;
 if(pg===9&&i===2)return ['HELD','8 SEC'][v]||v;
 if(pg===9&&i===3)return ['CLEAN','TAPE'][v]||v;
 if(pg===10&&i===0)return ['UP','DOWN','UP+DN','RANDOM','PLAYED'][v]||v;
 if(pg===10&&i===1)return ['1/4','1/8','1/8T','1/16','1/16T','1/32'][v]||v;
 return String(v);
}
function onMessage(m){
 if(m.type==='error'){clearTimeout(audioTimeout);status(m.message);starting=false;$('#start').disabled=false;if(m.fatal){ready=false;$('#start').textContent='Recarregar motor';ctx?.close();ctx=null;node=null;}return}
 if(m.type==='ready'){clearTimeout(audioTimeout);ready=true;starting=false;$('#start').disabled=false;$('#start').textContent='Ⅱ Pausar áudio';$('#screenCover').style.display='none';status('Áudio ativo · dois motores em 44,1 kHz');document.body.dataset.ready='true';return}
 if(m.type!=='frame')return;
 if(m.fb){for(let i=0;i<m.fb.length;i++){const p=m.fb[i],j=i*4;pixels.data[j]=((p>>11)&31)*255/31;pixels.data[j+1]=((p>>5)&63)*255/63;pixels.data[j+2]=(p&31)*255/31;pixels.data[j+3]=255}g.putImageData(pixels,0,0)}
 if(m.state){lastState=m.state;const pg=m.state[0];$('#screenState').textContent=(PAGES[pg]||'PLAY')+' · '+(SOUNDS[m.state[1]]||'FLUTE');$('#volume').value=m.state[17];$('#volumeVal').textContent=Math.round(m.state[17]/1023*100)+'%';
  for(const [id,on] of [[6,m.state[4]],[16,m.state[6]],[17,m.state[7]],[1,m.bank]])$(`[data-btn="${id}"]`)?.setAttribute('aria-pressed',!!on);
  $$('#pages button').forEach((b,i)=>{b.setAttribute('aria-pressed',i===pg);b.disabled=i>=(m.pageCount||12);});
  const wraps=$$('.knob-wrap');for(let i=0;i<4;i++){wraps[i].querySelector('label').textContent='K'+(i+1)+' · '+(PARAMS[pg]?.[i]||'PARÂMETRO');wraps[i].querySelector('output').textContent=parameterValue(pg,i,m.params[i]);wraps[i].querySelector('.knob').setAttribute('aria-valuetext',PARAMS[pg]?.[i]+' '+parameterValue(pg,i,m.params[i]));}
  wraps[4].querySelector('output').textContent=PAGES[pg];wraps[5].querySelector('output').textContent=m.tempo+' BPM';wraps[6].querySelector('output').textContent=SOUNDS[m.state[1]];
  $('#faderBank').textContent='Banco '+(m.bank?'B':'A');const labels=m.bank?['AGE','DRIVE','DELAY','PLATE']:['LYR MIX','TONE','WOW','FLUTTER'];$$('.fader').forEach((f,i)=>{f.querySelector('small').textContent='F'+(i+1)+' · '+labels[i];f.querySelector('input').setAttribute('aria-label','Fader '+(i+1)+' '+labels[i]);});
  const equal=m.states.A.slice(1,4).every((x,i)=>x===m.states.B[i+1]);$('#testState').textContent=equal?'A/B · mesmos timbres e layer':'A/B · estados diferentes nesta operação';
 }
 if(m.pads){$$('.pad').forEach((b,i)=>{const c=m.pads[i],r=(c>>8)&255,green=(c>>16)&255,blue=c&255;b.style.background=`rgb(${r},${green},${blue})`;b.style.color=green>90?'#142725':'#f1d2ad';b.querySelector('small').textContent=padNames[i];b.disabled=false;b.setAttribute('aria-label',`Pad ${i+1} ${padNames[i]}`);b.setAttribute('aria-pressed',false);});}
 if(m.scenes){const [on,pin,mask,selected]=m.scenes;$$('.pad').forEach((b,i)=>{const label=!on?padNames[i]:i<8?`S${String(i+1).padStart(2,'0')} · ${mask&(1<<i)?'GRAVADA':'VAZIA'}`:i===8?(pin?'FIXADO · SAIR':'FIXAR CENAS'):'';b.querySelector('small').textContent=label;b.disabled=!!on&&i>8;b.setAttribute('aria-label',`Pad ${i+1} ${label}`);b.setAttribute('aria-pressed',!!on&&(i<8?i===selected:i===8&&!!pin))});if(on)$('#screenState').textContent+=(pin?' · PRESETS':' · CENAS')+(selected<8?' S'+String(selected+1).padStart(2,'0'):'')}
 for(const s of m.stores||[])try{if(s.settings)localStorage.setItem(storageKey(s.hash,'boot'),JSON.stringify(s.settings));if(s.sceneStore)localStorage.setItem(storageKey(s.hash,'scenes'),JSON.stringify(s.sceneStore));}catch{status('SAVE indisponível: o navegador bloqueou o armazenamento local.');}
 phrasePlaying=m.phrase;$('#phrase').textContent=m.phrase?'■ Parar frase':'▷ Frase de teste';$('#phrase').setAttribute('aria-pressed',m.phrase);
 const db=Number.isFinite(m.level)?m.level:-80;$('#meter i').style.width=Math.max(0,Math.min(100,(db+60)/60*100))+'%';$('#level').textContent=db<=-80?'−∞ dB':db.toFixed(1)+' dB';$('#meter').classList.toggle('clipping',m.clips>0);$('#meter').title=m.clips?'Picos no limite: '+m.clips:'Nível de pico';
}
function stored(hash,kind){try{return JSON.parse(localStorage.getItem(storageKey(hash,kind)))}catch{return null}}
function payload(clean=false){return Object.fromEntries(['A','B'].map(id=>[id,{wasm:builds[id].wasm,hash:builds[id].hash,settings:clean?null:stored(builds[id].hash,'boot'),sceneStore:clean?null:stored(builds[id].hash,'scenes')}]));}
async function loadPair(clean=false){releaseAll();send({type:'panic'});ready=false;starting=true;$('#start').disabled=true;status('Alinhando os motores…');$('#sustain').checked=false;$('#pitch').value=535;$('#mod').value=620;$$('.fader input').forEach(f=>f.value=0);node.port.postMessage({type:'load-pair',builds:payload(clean)});audioTimeout=setTimeout(()=>{if(starting){status('O motor não respondeu. Tente iniciar novamente.');starting=false;ready=false;ctx?.close();ctx=null;node=null;$('#start').disabled=false;$('#start').textContent='Tentar novamente';}},15000);}
$('#start').onclick=async()=>{
 if(starting)return;
 if(ready&&ctx){if(ctx.state==='running'){panic();await ctx.suspend();$('#start').textContent='▶ Retomar áudio';status('Áudio pausado')}else{await ctx.resume();$('#start').textContent='Ⅱ Pausar áudio';status('Áudio ativo · 44,1 kHz')}return}
 starting=true;$('#start').disabled=true;status('Carregando instrumento…');
 try{
  await prepared;if(!builds.A||!builds.B)throw Error('Não foi possível preparar as builds. Recarregue a página.');
  if(ctx)await ctx.close();ctx=new AudioContext({sampleRate:44100,latencyHint:'interactive'});await ctx.resume();
  if(ctx.sampleRate!==44100)throw Error('Este navegador não disponibilizou áudio em 44,1 kHz. Tente Chrome ou Edge.');
  if(!ctx.audioWorklet)throw Error('Áudio indisponível neste contexto. Abra o simulador pelo endereço HTTPS em um navegador compatível.');await ctx.audioWorklet.addModule('worklet.js?v=ap8');
  node=new AudioWorkletNode(ctx,'smktron',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2]});node.connect(ctx.destination);node.port.onmessage=e=>onMessage(e.data);await loadPair();
 }catch(e){status(e.message);$('#start').disabled=false;starting=false;ready=false;if(ctx)await ctx.close();ctx=null;node=null}
};
$('#phrase').onclick=()=>{if(!ready||ctx.state!=='running'){status('Inicie ou retome o áudio para tocar a frase.');return}releaseAll();$('#sustain').checked=false;send({type:'phrase',velocity:+$('#velocity').value});};
$('#restart').onclick=async()=>{if(!ready){status('Inicie o áudio para reiniciar o teste.');return}await loadPair(true);};
function showBuilds(){
 for(const id of ['A','B']){const b=builds[id];if(!b)continue;$('#buildName'+id).textContent=b.name;$('#buildNote'+id).textContent=b.note;$('#buildHash'+id).textContent=b.hash.slice(0,12);$('#buildHash'+id).title='SHA-256 '+b.hash;$('#listen'+id+' span').textContent=b.name.replace('0.3.0-','').toUpperCase();}
 $('#pairSummary').textContent='A · '+(builds.A?.name||'…')+' / B · '+(builds.B?.name||'…');$('#activeBuild').textContent=active+' / '+(builds[active]?.name||'…');
}
for(const id of ['A','B'])$('#listen'+id).onclick=()=>{active=id;send({type:'active',id});for(const n of ['A','B'])$('#listen'+n).setAttribute('aria-pressed',n===id);showBuilds();};
async function database(){return new Promise((resolve,reject)=>{const req=indexedDB.open('smktron-lab-builds',1);req.onupgradeneeded=()=>req.result.createObjectStore('builds');req.onsuccess=()=>resolve(req.result);req.onerror=()=>reject(req.error);});}
async function cache(id,build){const db=await database();try{return await new Promise((resolve,reject)=>{const tx=db.transaction('builds',build===undefined?'readonly':'readwrite'),store=tx.objectStore('builds');const req=build===undefined?store.get(id):build===null?store.delete(id):store.put(build,id);let result;req.onsuccess=()=>result=req.result;tx.oncomplete=()=>resolve(result);tx.onerror=()=>reject(tx.error);tx.onabort=()=>reject(tx.error);});}finally{db.close();}}
async function verify(wasm,name,note){
 if(!crypto.subtle)throw Error('Abra o endereço HTTPS privado para carregar as builds e ativar o áudio.');
 const module=await WebAssembly.compile(wasm);validateModule(module);const e=(await WebAssembly.instantiate(module,{})).exports;if(e.web_sample_rate()!==44100)throw Error('Build incompatível: áudio precisa ser 44,1 kHz.');e.web_boot();e.web_render(128);
 const hash=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',wasm)),x=>x.toString(16).padStart(2,'0')).join('');return {wasm,name,note,hash};
}
async function bundled(id){const d=defaultBuilds[id],r=await fetch(d.url);if(!r.ok)throw Error('Não foi possível carregar '+d.name);return verify(await r.arrayBuffer(),d.name,d.note);}
for(const id of ['A','B']){
 $('#file'+id).onchange=async e=>{const f=e.target.files[0];if(!f)return;if(starting){status('Aguarde o motor terminar de carregar.');e.target.value='';return}if(!f.name.toLowerCase().endsWith('.wasm')||f.size>16*1024*1024){status('Escolha uma build .wasm de até 16 MB.');e.target.value='';return}
  try{status('Verificando '+f.name+'…');const b=await verify(await f.arrayBuffer(),f.name.replace(/\.wasm$/i,''),'Build local · importada neste navegador');builds[id]=b;showBuilds();if(node)await loadPair(true);try{await cache(id,b);}catch{status('Build pronta nesta sessão; o navegador impediu sua preservação.');}if(!node)status('Build carregada. Inicie o áudio para testar.');}catch(err){status(err.message);}finally{e.target.value='';}
 };
 $('#restore'+id).onclick=async()=>{if(starting)return;try{builds[id]=await bundled(id);showBuilds();await cache(id,null).catch(()=>{});if(node)await loadPair(true);else status('Build de referência restaurada.');}catch(e){status(e.message);}};
}
prepared=(async()=>{
 try{for(const id of ['A','B']){let b;try{b=await cache(id);if(b)b=await verify(b.wasm,b.name,b.note);}catch{}builds[id]=b||await bundled(id);}showBuilds();}
 catch(e){status(e.message);}
})();
function attachMidi(){
 if(midiPort)midiPort.onmidimessage=null;midiPort=midiAccess?.inputs.get($('#midiInput').value);
 if(midiPort)midiPort.onmidimessage=e=>{if(ready)send({type:'midi',data:Array.from(e.data)})};
}
function midiList(){
 const old=$('#midiInput').value;$('#midiInput').replaceChildren();for(const p of midiAccess.inputs.values()){const o=document.createElement('option');o.value=p.id;o.textContent=p.name||'Entrada MIDI';$('#midiInput').append(o)}
 $('#midiInput').hidden=!midiAccess.inputs.size;if(old&&midiAccess.inputs.has(old))$('#midiInput').value=old;attachMidi();status(midiPort?'MIDI conectado: '+midiPort.name:'MIDI autorizado · conecte um teclado.');
}
$('#midi').onclick=async()=>{try{if(!navigator.requestMIDIAccess)throw Error('MIDI não disponível neste navegador. Use Chrome ou Edge.');panic();midiAccess=await navigator.requestMIDIAccess({sysex:false});midiAccess.onstatechange=midiList;midiList()}catch(e){status('MIDI: '+e.message)}};$('#midiInput').onchange=()=>{panic();attachMidi()};
$('#reset').onclick=async()=>{if(confirm('Apagar o SAVE e as cenas da build '+builds[active].name+' neste navegador?')){localStorage.removeItem(storageKey(builds[active].hash,'boot'));localStorage.removeItem(storageKey(builds[active].hash,'scenes'));if(node)await loadPair();else status('SAVE e cenas apagados.');}};
