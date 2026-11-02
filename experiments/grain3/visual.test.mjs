import{test}from'node:test';import assert from'node:assert/strict';import{readFileSync,writeFileSync,mkdirSync}from'node:fs';
const wasm=readFileSync('dist/grain3-preview/mel.wasm'),reference=readFileSync('dist/grain2-preview/mel.wasm');
const run=(e,ms=50)=>{for(let i=0;i<Math.ceil(ms*44.1/128);i++)e.web_render(128)};
const btn=(e,k,ms=50)=>{e.web_button(k,1);run(e,ms);e.web_button(k,0);run(e)};
async function boot(w=wasm){const e=(await WebAssembly.instantiate(w,{})).instance.exports;e.web_boot();run(e,4000);return e}
const fb=(e,y=0,h=240)=>new Uint8Array(e.memory.buffer,e.web_screen()+y*240*2,h*240*2).slice();
const rgb=(e,p=9)=>{const c=e.web_pad_color(p);return[(c>>8)&255,(c>>16)&255,c&255]};
function grain(e){btn(e,6);e.web_grain_style(1);e.web_grain_on(1);run(e,100)}
test('Light Dark Zoom: grain motion with sound only, fixed controls; stop when OFF and after note decay',async()=>{
 mkdirSync('experiments/grain3/captures',{recursive:true});
 for(const theme of[0,1,2]){const e=await boot();btn(e,10);btn(e,10);e.web_enc(1,theme);run(e);grain(e);
 const [y,h]=theme===2?[210,12]:[66,64];const idle=fb(e,y,h);run(e,400);assert.deepEqual(fb(e,y,h),idle,'idle is still');
 e.web_key(12,100);run(e,150);const a=fb(e,y,h),controls=theme===2?fb(e,66,128):fb(e,200,40);run(e,260);assert.notDeepEqual(fb(e,y,h),a);assert.deepEqual(theme===2?fb(e,66,128):fb(e,200,40),controls,'parameter labels and values remain fixed');
 for(let i=0;i<30;i++){run(e,80);writeFileSync(`experiments/grain3/captures/grain-${theme}-${i}.rgb565`,fb(e));writeFileSync(`experiments/grain3/captures/grain-${theme}-${i}.json`,JSON.stringify(rgb(e)))}
 e.web_grain_on(0);run(e,120);const off=fb(e,y,h);run(e,400);assert.deepEqual(fb(e,y,h),off);e.web_grain_on(1);e.web_key(12,0);run(e,5000);const stopped=fb(e,y,h);run(e,400);assert.deepEqual(fb(e,y,h),stopped);
 }
});
test('pad 10: warm dim OFF, steady coral idle, rounded multicolour pulse while sounding, bypass steady',async()=>{
 const e=await boot();btn(e,10);btn(e,10);e.web_enc(0,-100);run(e);assert.deepEqual(rgb(e),[16,4,2]);grain(e);assert.deepEqual(rgb(e),[144,48,24]);
 e.web_key(12,100);run(e,100);const levels=[];for(let i=0;i<16;i++){run(e,80);const c=rgb(e);assert(c[0]>=c[1]&&c[0]>=c[2]);levels.push(c.join(','))}assert(new Set(levels).size>6);
 btn(e,14,700);assert.deepEqual(rgb(e),[64,18,9]);run(e,500);assert.deepEqual(rgb(e),[64,18,9]);btn(e,14,700);e.web_grain_set(3,0);run(e,100);assert.deepEqual(rgb(e),[64,18,9]);
});
test('global brightness scales grain LED and other pad modes keep their own colours',async()=>{
 const e=await boot();grain(e);btn(e,10);btn(e,10);e.web_enc(0,-100);run(e);assert.deepEqual(rgb(e),[144,48,24]);e.web_enc(0,1);run(e);assert.deepEqual(rgb(e),[108,36,18]);
 const b=await boot(reference);for(const x of[e,b]){x.web_chord_mode(1);run(x)}assert.deepEqual(rgb(e),rgb(b),'pad 10 is HOLD in chords, not GRAIN');
 for(const x of[e,b]){x.web_chord_mode(0);run(x);btn(x,0)}assert.deepEqual(rgb(e),rgb(b),'favorites owns pad 10');
});
test('new animation and legacy UI cleanup leave GR2 musical PCM and persisted bytes unchanged',async()=>{
 const a=await boot(),b=await boot(reference);for(const e of[a,b]){grain(e);for(const k of[0,5,12,16,19,24,31,36])e.web_key(k,100);run(e,200)}
 for(let i=0;i<900;i++){
  if(i===200)for(const e of[a,b])e.web_grain_style(3);
  if(i===400)for(const e of[a,b])e.web_midi(0xB0,1,75);
  if(i===500)for(const e of[a,b])e.web_midi(0xB0,1,0);
  if(i===650)for(const e of[a,b])e.web_grain_on(0);
  for(const e of[a,b])e.web_render(128);
  for(const ch of['web_out_l','web_out_r'])assert.deepEqual(new Float32Array(a.memory.buffer,a[ch](),128),new Float32Array(b.memory.buffer,b[ch](),128));
 }
 for(const e of[a,b]){btn(e,11);btn(e,0);e.web_pad(0,100);run(e,1100);e.web_pad(0,0);run(e)}
 for(const f of['web_settings','web_scene_flash','web_chord_flash']){const n=f==='web_settings'?a.web_settings_len():8192;assert.deepEqual(new Uint8Array(a.memory.buffer,a[f](),n),new Uint8Array(b.memory.buffer,b[f](),n),f)}
});
