import{test}from'node:test';import assert from'node:assert/strict';import{readFileSync}from'node:fs';import vm from'node:vm';
test('desktop layer-add replays the engine pad gesture; solo, editor and mode guards stay intact',async()=>{
 let Klass;const messages=[];vm.runInNewContext(readFileSync('web/worklet.js','utf8'),{AudioWorkletProcessor:class{constructor(){this.port={postMessage:m=>messages.push(m)}}},registerProcessor:(name,k)=>{assert.equal(name,'smktron');Klass=k},WebAssembly,Uint8Array,Uint16Array,Float32Array,currentTime:1,console});
 const p=new Klass();await p.message({type:'load',wasm:readFileSync('web/mel.wasm')});assert(p.ex);const e=p.ex,run=(ms=80)=>{for(let i=0;i<Math.ceil(ms*44.1/128);i++)e.web_render(128)};
 run(1800);await p.message({type:'layer-add',k:5});assert.equal(e.web_mm_state(13),1);
 await p.message({type:'mode',value:1});run(400);await p.message({type:'pad-bank'});run(40);assert.equal(e.web_mm_state(9),1);
 await p.message({type:'pad',k:2,vel:100});run();await p.message({type:'pad',k:2,vel:0});run();assert.equal(e.web_mm_state(13),4);
 for(const k of[0,5,15]){await p.message({type:'layer-add',k});run();}assert.equal(e.web_mm_state(13),0x8025);assert.equal(e.web_mm_state(14),4);
 await p.message({type:'layer-add',k:5});assert.equal(e.web_mm_state(13),0x8025);
 await p.message({type:'pad',k:8,vel:100});run();await p.message({type:'pad',k:8,vel:0});run();assert.equal(e.web_mm_state(13),256);
 e.web_button(13,1);run(40);e.web_button(13,0);run(40);assert.equal(e.web_mm_state(5),1);await p.message({type:'layer-add',k:1});run();assert.equal(e.web_mm_state(13),256);
 const n=e.web_mm_state(13);for(const k of[-1,16,1.5,'1'])await p.message({type:'layer-add',k});assert.equal(e.web_mm_state(13),n);
 const app=readFileSync('web/app.js','utf8');assert(app.includes("send({type:'layer-add',k:i})")&&app.includes('e.shiftKey&&currentMm[0]&&currentMm[9]'));
});
