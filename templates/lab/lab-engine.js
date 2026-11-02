// SPDX-License-Identifier: GPL-3.0-only
// Shared by the AudioWorklet and the regression suite. No browser mocks in DSP tests.
export const REQUIRED=['memory','web_boot','web_render','web_out_l','web_out_r','web_key','web_pad','web_button','web_enc','web_smk_master','web_fader','web_wheel','web_midi','web_panic','web_probe','web_screen','web_pad_color','web_settings','web_settings_len','web_settings_set_len','web_settings_writes','web_now_ms','web_sample_rate','web_page_param','web_fader_bank'];
export function validateModule(module){
 if(WebAssembly.Module.imports(module).length)throw Error('Build incompatível: o simulador requer um WASM independente, sem imports.');
 const names=new Set(WebAssembly.Module.exports(module).map(x=>x.name));
 const missing=REQUIRED.filter(x=>!names.has(x));
 if(missing.length)throw Error('Build incompatível com o simulador SMKTRON: faltam '+missing.join(', ')+'.');
}
const bytesOK=(a,n)=>Array.isArray(a)&&a.length>0&&a.length<=n&&a.every(v=>Number.isInteger(v)&&v>=0&&v<=255);
export class LabEngine{
 constructor(){this.slots={};this.active='A';this.clock=0;this.sequence=[];this.phrase=false;this.phraseEnd=0;this.clips={A:0,B:0};this.level=-Infinity;this.navTargets={};this.navDown={};this.muted=false;}
 async loadPair(builds){
  // Prepare both before replacing either; a rejected file leaves the running pair intact.
  const pair={};
  for(const id of ['A','B']){
   const b=builds[id],module=b.module||await WebAssembly.compile(b.wasm);validateModule(module);
   const e=(await WebAssembly.instantiate(module,{})).exports;
   if(e.web_sample_rate()!==44100)throw Error('Frequência de áudio incompatível: é necessário um motor de 44,1 kHz.');
   if(bytesOK(b.settings,256)){new Uint8Array(e.memory.buffer,e.web_settings(),b.settings.length).set(b.settings);e.web_settings_set_len(b.settings.length);}
   if(e.web_scene_flash&&bytesOK(b.sceneStore,8192)&&b.sceneStore.length===8192)new Uint8Array(e.memory.buffer,e.web_scene_flash(),8192).set(b.sceneStore);
   e.web_master?.(700);e.web_boot();
   pair[id]={e,hash:b.hash,writes:e.web_settings_writes(),sceneWrites:e.web_scene_writes?.()||0};
  }
  this.slots=pair;this.clock=0;this.sequence=[];this.phrase=false;this.clips={A:0,B:0};this.level=-Infinity;this.navTargets={};this.navDown={};this.muted=false;
 }
 control(m){
  if((m.type==='key'&&m.vel>0)||(m.type==='midi'&&(m.data[0]&240)===144&&m.data[2]>0))this.muted=false;
  if(m.type==='page'){for(const [id,{e}] of Object.entries(this.slots)){const count=e.web_page_count?.()||12;if(m.page<count)this.navTargets[id]=m.page;}return;}
  if(m.type==='active'){if(this.slots[m.id])this.active=m.id;return;}
  if(m.type==='phrase'){if(this.phrase)this.stop();else this.startPhrase(m.velocity||100);return;}
  if(m.type==='panic'){this.stop();return;}
  for(const {e} of Object.values(this.slots))switch(m.type){
   case 'key':e.web_key(m.k,m.vel);break;
   case 'pad':e.web_pad(m.k,m.vel);break;
   case 'pad-cancel':e.web_scene_cancel_pad?e.web_scene_cancel_pad(m.k):e.web_pad(m.k,0);break;
   case 'save-cancel':e.web_scene_cancel_save?e.web_scene_cancel_save():e.web_button(11,0);break;
   case 'button':e.web_button(m.id,m.down?1:0);break;
   case 'enc':if(m.id<4&&(e.web_probe(0)===12||this.slots[this.active].e.web_probe(0)===12)&&e.web_probe(0)!==this.slots[this.active].e.web_probe(0))break;e.web_enc(m.id,m.n);break;
   
   case 'master':e.web_smk_master(m.value);break;
   case 'fader':e.web_fader(m.id,m.value);break;
   case 'wheel':e.web_wheel(m.id,m.value);break;
   case 'midi':e.web_midi(m.data[0]||0,m.data[1]||0,m.data[2]||0);break;
  }
 }
 stop(){this.sequence=[];this.phrase=false;this.muted=true;for(const {e} of Object.values(this.slots))e.web_panic();}
 startPhrase(velocity){
  this.stop();this.muted=false;this.phrase=true;const start=this.clock;
  // Identical sample-clock scheduling for both engines. C3, C4, G4, C5 then Cm.
  const add=(ms,k,vel)=>this.sequence.push({at:start+Math.round(ms*44.1),k,vel});
  for(const [ms,k] of [[0,0],[600,12],[1200,19],[1800,24]]){add(ms,k,velocity);add(ms+400,k,0);}
  for(const k of [12,15,19]){add(2500,k,velocity);add(4200,k,0);}
  this.sequence.sort((a,b)=>a.at-b.at);this.phraseEnd=start+Math.round(6200*44.1);
 }
 render(n){
  while(this.sequence.length&&this.sequence[0].at<=this.clock){const m=this.sequence.shift();for(const {e} of Object.values(this.slots))e.web_key(m.k,m.vel);}
  let selected=null;
  for(const [id,{e}] of Object.entries(this.slots)){
   if(this.navDown[id]){e.web_button(3,0);this.navDown[id]=false;}else if(this.navTargets[id]!==undefined&&e.web_probe(0)!==this.navTargets[id]){e.web_button(3,1);this.navDown[id]=true;}else delete this.navTargets[id];
   e.web_render(n);const l=new Float32Array(e.memory.buffer,e.web_out_l(),n),r=new Float32Array(e.memory.buffer,e.web_out_r(),n);
   let peak=0;for(let i=0;i<n;i++){if(!Number.isFinite(l[i])||!Number.isFinite(r[i]))throw Error('O motor '+id+' produziu áudio inválido.');peak=Math.max(peak,Math.abs(l[i]),Math.abs(r[i]));}
   if(peak>=.999)this.clips[id]++;if(id===this.active){selected=[l,r];this.level=peak>0?20*Math.log10(peak):-Infinity;}
  }
  this.clock+=n;if(this.phrase&&this.clock>=this.phraseEnd)this.phrase=false;if(this.muted){this.level=-Infinity;return [new Float32Array(n),new Float32Array(n)];}return selected;
 }
 frame(){
  const slot=this.slots[this.active];if(!slot)return null;const e=slot.e,pg=e.web_probe(0);
  const states={};for(const [id,s] of Object.entries(this.slots))states[id]=Array.from({length:39},(_,i)=>s.e.web_probe(i));
  const msg={type:'frame',pageCount:e.web_page_count?.()||12,active:this.active,fb:new Uint16Array(e.memory.buffer,e.web_screen(),57600).slice(),pads:Array.from({length:16},(_,i)=>e.web_pad_color(i)),state:states[this.active],states,params:Array.from({length:4},(_,i)=>e.web_page_param(pg,i)),bank:e.web_fader_bank(),tempo:e.web_page_param(9,0),phrase:this.phrase,level:this.level,clips:this.clips[this.active],stores:[]};
  if(e.web_scene_mode)msg.scenes=[e.web_scene_mode(),e.web_scene_pin(),e.web_scene_mask(),e.web_scene_selected()];
  for(const [id,s] of Object.entries(this.slots)){
   const x=s.e,save={id,hash:s.hash};let changed=false;
   if(s.writes!==x.web_settings_writes()){s.writes=x.web_settings_writes();const n=x.web_settings_len();if(n>0&&n<=256){save.settings=Array.from(new Uint8Array(x.memory.buffer,x.web_settings(),n));changed=true;}}
   if(x.web_scene_writes&&s.sceneWrites!==x.web_scene_writes()){s.sceneWrites=x.web_scene_writes();save.sceneStore=Array.from(new Uint8Array(x.memory.buffer,x.web_scene_flash(),8192));changed=true;}
   if(changed)msg.stores.push(save);
  }
  return msg;
 }
}
