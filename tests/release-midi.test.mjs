import{test}from'node:test';import assert from'node:assert/strict';import{readFileSync,existsSync}from'node:fs';import{createHash,webcrypto}from'node:crypto';import{resolve}from'node:path';
globalThis.crypto??=webcrypto;
const sha=b=>createHash('sha256').update(b).digest('hex');
const manifest=JSON.parse(readFileSync('web/install/manifest.json'));
test('approved package, source, manifest and installer agree; model/resume checks remain enforced',async()=>{
 const {PINS,validate,modeFor,readPending}=await import('../web/install/service.js');const raw=readFileSync('web/install/'+PINS.rc.url);
 assert.equal(sha(raw),'9249b40ad1c7c30f564790a6be90a5561ff0cee29ac5351c331f60a2fd3d7545');assert.equal(sha(raw),manifest.firmware_sha256);assert.equal(manifest.hardware_tested,true);assert.equal((await validate(raw,'rc')).product,'SMK-37 Pro_460');
 for(const n of[16,453,458,459,460])assert.equal(modeFor('rc',{model:'SMK-37 Pro',text:'SMK-37 Pro_'+String(n).padStart(3,'0')},null),'install');
 for(const model of['FM-1','SMK-37 Elite','MKE-P37'])assert.throws(()=>modeFor('rc',{model,text:model+'_016'},null));
 assert.throws(()=>modeFor('rc',{model:'ota-SMK-37 Pro',text:'ota-SMK-37 Pro_900'},null));assert.equal(modeFor('rc',{model:'ota-SMK-37 Pro',text:'ota-SMK-37 Pro_900'},{kind:'rc',source:'SMK-37 Pro_453'}),'resume');assert.equal(readPending({getItem:()=>'{"kind":"rc","sha":"wrong"}'}),null);
 const bad=Buffer.from(raw);bad[22000]^=1;await assert.rejects(validate(bad,'rc'));
});
const run=(e,ms=120)=>{for(let i=0;i<Math.ceil(ms*44.1/128);i++)e.web_render(128)},button=(e,k)=>{e.web_button(k,1);run(e,50);e.web_button(k,0);run(e)},drain=e=>{const a=[];while(e.web_mm_tx_count())a.push(e.web_mm_tx_take());return a.map(n=>[n&255,n>>8&255,n>>16&255]);};
test('published simulator has exclusive MIDI, physical navigation, keys, K5, pad CC and return',async()=>{
 const e=(await WebAssembly.instantiate(readFileSync('web/mel.wasm'),{})).instance.exports;e.web_boot();run(e,1800);assert.equal(e.web_page_count(),15);e.web_midi(0x90,60,127);run(e);assert.equal(e.web_mm_tx_count(),0);
 for(let i=0;i<4;i++)button(e,10);assert.equal(e.web_probe(0),14);e.web_enc(0,1);run(e,400);assert.equal(e.web_mm_state(0),1);drain(e);
 e.web_key(0,100);run(e);assert(drain(e).some(m=>m[0]===0x90&&m[1]===48&&m[2]===100));e.web_key(0,0);run(e);assert(drain(e).some(m=>m[0]===0x80));
 e.web_enc(4,6);run(e);assert(drain(e).some(m=>m[0]===0xB0));e.web_pad(0,100);run(e);assert(drain(e).length);e.web_pad(0,0);run(e);drain(e);
 assert(new Float32Array(e.memory.buffer,e.web_out_l(),128).every(x=>x===0));button(e,10);assert.equal(e.web_mm_state(6),1);e.web_enc(0,-1);run(e,500);assert.equal(e.web_mm_state(0),0);drain(e);e.web_key(1,100);run(e);assert.equal(e.web_mm_tx_count(),0);
});
test('manual chapter, images, local links and all firmware sources are included',()=>{
 const html=readFileSync('web/manual/index.html','utf8'),ids=[...html.matchAll(/\bid="([^"]+)"/g)].map(m=>m[1]);assert.equal(ids.length,new Set(ids).size);assert(ids.includes('midi-mode'));
 for(const token of['K5','ABS','REL','MIDI PANIC','English — MIDI MODE','CC64','quatro mapas','GLOBE'])assert(html.includes(token),token);
 assert(!html.includes('O SMKTRON recebe notas MIDI'));assert(html.includes('GRAIN / Grãos de fita'));
 for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){const u=m[1].split('?')[0].split('#')[0];if(!u||/^https?:/.test(u))continue;assert(existsSync(resolve('web/manual',u)),u);}
 for(const file of['firmware/firmware/src/mel.c','firmware/firmware/hal/smk_uart_tx.h','experiments/midi/build.py','manual/midi.html','web/source.zip','CHANGELOG.md'])assert(existsSync(file),file);
 const captures=JSON.parse(readFileSync('web/manual/midi-captures.json'));assert.equal(captures.wasm_sha256,sha(readFileSync('web/mel.wasm')));assert.equal(captures.captures.length,9);
});
