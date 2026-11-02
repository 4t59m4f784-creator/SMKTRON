import{test}from'node:test';import assert from'node:assert/strict';import{readFileSync,writeFileSync,mkdirSync}from'node:fs';
const wasm=readFileSync('dist/grain-preview/mel.wasm'),reference=readFileSync('dist/zoom-preview/mel.wasm');
const run=(e,ms=50)=>{for(let i=0;i<Math.ceil(ms*44.1/128);i++)e.web_render(128)};
const btn=(e,k,ms=50)=>{e.web_button(k,1);run(e,ms);e.web_button(k,0);run(e)};
const pad=(e,k,ms=50)=>{e.web_pad(k,100);run(e,ms);e.web_pad(k,0);run(e)};
const keys=(e,ns,on)=>{for(const n of ns)e.web_key(n,on?100:0);run(e)};
async function boot(w=wasm,settings,flash){const e=(await WebAssembly.instantiate(w,{})).instance.exports;if(settings){new Uint8Array(e.memory.buffer,e.web_settings(),settings.length).set(settings);e.web_settings_set_len(settings.length)}if(flash)new Uint8Array(e.memory.buffer,e.web_chord_flash(),8192).set(flash);e.web_boot();run(e,1700);return e;}
const samples=e=>new Float32Array(e.memory.buffer,e.web_out_l(),128).slice();
// Existing AT2/Zoom fixed-point FX retain a small DC floor; assess remaining AC, not DC.
const quiet=e=>{const a=samples(e),mean=a.reduce((s,x)=>s+x,0)/a.length;return a.every(x=>Math.abs(x-mean)<.002)};
test('MIX zero is bit-exact ZM1 for all tapes, normal/layer; no grain decoding while bypassed',async()=>{
 for(let st=0;st<4;st++)for(const ly of[0,1]){
  const a=await boot(),b=await boot(reference);
  for(const e of[a,b]){e.web_enc(7,st);run(e);if(ly){btn(e,8);e.web_enc(0,4);run(e)}keys(e,[5,12,16,19,24],true)}
  for(let i=0;i<90;i++){a.web_render(128);b.web_render(128);for(const ch of['l','r'])assert.deepEqual(new Float32Array(a.memory.buffer,a['web_out_'+ch](),128),new Float32Array(b.memory.buffer,b['web_out_'+ch](),128));}
  assert.equal(a.web_grain_frames(),0);assert.equal(a.web_grain_spawns(),0);
 }
});
test('ARP button opens GRAIN and never steals keyboard notes; physical knobs clamp to documented ranges',async()=>{
 const e=await boot();btn(e,6);assert.equal(e.web_probe(0),10);assert.equal(e.web_probe(4),0);
 keys(e,[12,16,19],true);for(const n of[60,64,67])assert.equal(e.web_voice_gate(0,n),1);
 for(const[k,lo,hi]of[[0,20,400],[1,0,100],[2,15,100],[3,0,100]]){e.web_enc(k,-1000);run(e);assert.equal(e.web_page_param(10,k),lo);e.web_enc(k,1000);run(e);assert.equal(e.web_page_param(10,k),hi)}
 btn(e,6);assert.equal(e.web_probe(4),0);for(const n of[60,64,67])assert.equal(e.web_voice_gate(0,n),1);
});
test('granulation changes audio; extreme controls/all tapes/layer/12 notes produce finite bounded PCM and release',async()=>{
 for(let st=0;st<4;st++){
  const e=await boot();e.web_enc(7,st);run(e);btn(e,8);e.web_enc(0,4);run(e);
  keys(e,[0,3,5,7,10,12,15,17,19,22,24,31],true);const dry=samples(e);
  e.web_grain_set(3,100);let differs=false,energy=0,peak=0;
  for(let i=0;i<1600;i++){
   if(i%200===0){e.web_grain_set(0,i%400?400:20);e.web_grain_set(1,i%400?0:100);e.web_grain_set(2,i%400?15:100)}
   e.web_render(128);const a=samples(e);if(i===100)differs=a.some((x,j)=>x!==dry[j]);
   for(const x of a){assert(Number.isFinite(x));assert(Math.abs(x)<=1.01);energy+=x*x;peak=Math.max(peak,Math.abs(x))}
  }
  assert(differs);assert(energy>1);assert(e.web_grain_spawns()>10);assert(e.web_grain_frames()>0);
  keys(e,[0,3,5,7,10,12,15,17,19,22,24,31],false);run(e,5000);assert(quiet(e));
 }
});
test('sequencer step recording, playback pitch order and timing match ZM1 after ARP removal',async()=>{
 const a=await boot(),b=await boot(reference);
 for(const e of[a,b]){btn(e,17);keys(e,[12,16],true);keys(e,[12,16],false);keys(e,[14,17],true);keys(e,[14,17],false);btn(e,17);e.web_enc(0,-14);run(e);btn(e,16)}
 for(let i=0;i<420;i++){a.web_render(128);b.web_render(128);for(const k of[6,7,8,9,38])assert.equal(a.web_probe(k),b.web_probe(k));assert.deepEqual(samples(a),samples(b));}
 a.web_grain_set(3,80);run(a,1000);assert.equal(a.web_probe(6),1);assert(a.web_grain_spawns()>0);btn(a,16);assert.equal(a.web_probe(6),0);
});
test('chord recording/global persistence and independent melody still work with granulation',async()=>{
 const e=await boot();e.web_chord_mode(1);pad(e,8);pad(e,0);keys(e,[12,16,19],true);keys(e,[12,16,19],false);
 assert.equal(e.web_chord_writes(),1);e.web_grain_set(3,80);pad(e,9);pad(e,0);keys(e,[24],true);run(e,500);
 assert.equal(e.web_voice_gate(1,60),1);assert.equal(e.web_voice_gate(0,72),1);assert(e.web_grain_spawns()>0);
 btn(e,6);assert.equal(e.web_probe(0),10);e.web_enc(3,-10);run(e);assert.equal(e.web_page_param(10,3),70);assert.equal(e.web_chord_state(0),1);assert.equal(e.web_voice_gate(1,60),1);
 const flash=new Uint8Array(e.memory.buffer,e.web_chord_flash(),8192).slice();const f=await boot(wasm,null,flash);assert.equal(f.web_chord_note(0,0),60);
 e.web_panic();run(e,6000);assert(quiet(e));
});
test('experimental grain controls do not change boot record size; saved boot/scene/chords remain isolated',async()=>{
 const a=await boot(),b=await boot(reference);assert.equal(a.web_current_state_size(),b.web_current_state_size());a.web_grain_set(3,85);btn(a,11);
 const save=new Uint8Array(a.memory.buffer,a.web_settings(),a.web_settings_len()).slice();const f=await boot(wasm,save);assert.equal(f.web_page_param(10,3),0,'grain intentionally session-only');assert.equal(f.web_output_level(),70);
 btn(a,0);pad(a,0,1100);assert(a.web_scene_mask()&1);a.web_grain_set(1,85);pad(a,0);assert.equal(a.web_page_param(10,1),85);assert.equal(a.web_chord_writes(),0);
});
test('all themes render GRAIN with actual firmware capture',async()=>{mkdirSync('experiments/grain/captures',{recursive:true});for(const t of[0,1,2]){const e=await boot();btn(e,10);btn(e,10);e.web_enc(1,t);run(e);btn(e,6);e.web_grain_set(3,75);keys(e,[12,16,19],true);run(e,150);writeFileSync('experiments/grain/captures/theme-'+t+'.rgb565',new Uint8Array(e.memory.buffer,e.web_screen(),115200));assert.equal(e.web_probe(0),10);}});
