// Render actual SMKTRON firmware framebuffers; no mock UI.
import{readFileSync,writeFileSync,mkdirSync}from'node:fs';import{deflateSync}from'node:zlib';import assert from'node:assert/strict';
const e=(await WebAssembly.instantiate(readFileSync('web/mel.wasm'),{})).instance.exports;
const run=(ms=80)=>{for(let i=0;i<Math.ceil(ms*44.1/128);i++)e.web_render(128)};
const btn=k=>{e.web_button(k,1);run();e.web_button(k,0);run()};
function page(p){for(let i=0;e.web_probe(0)!==p&&i<18;i++)btn(3);assert.equal(e.web_probe(0),p)}
function crc(b){let c=0xffffffff;for(const x of b){c^=x;for(let i=0;i<8;i++)c=(c>>>1)^((c&1)?0xedb88320:0)}return(c^0xffffffff)>>>0}
function chunk(t,b){const type=Buffer.from(t),size=Buffer.alloc(4),sum=Buffer.alloc(4);size.writeUInt32BE(b.length);sum.writeUInt32BE(crc(Buffer.concat([type,b])));return Buffer.concat([size,type,b,sum])}
function capture(name){const scale=3,w=240*scale;const pixels=new Uint16Array(e.memory.buffer,e.web_screen(),57600);const raw=Buffer.alloc((w*3+1)*w);
 for(let y=0;y<w;y++)for(let x=0;x<w;x++){const c=pixels[Math.floor(y/scale)*240+Math.floor(x/scale)],p=y*(w*3+1)+1+x*3;raw[p]=Math.round(((c>>11)&31)*255/31);raw[p+1]=Math.round(((c>>5)&63)*255/63);raw[p+2]=Math.round((c&31)*255/31)}
 const h=Buffer.alloc(13);h.writeUInt32BE(w);h.writeUInt32BE(w,4);h[8]=8;h[9]=2;
 writeFileSync('web/images/smktron-'+name+'.png',Buffer.concat([Buffer.from([137,80,78,71,13,10,26,10]),chunk('IHDR',h),chunk('IDAT',deflateSync(raw)),chunk('IEND',Buffer.alloc(0))]));}

import{copyFileSync}from'node:fs';import{createHash}from'node:crypto';
mkdirSync('web/images',{recursive:true});mkdirSync('web/manual/images',{recursive:true});
const records=[];
function shot(name){run(250);capture(name);copyFileSync('web/images/smktron-'+name+'.png','web/manual/images/'+name+'.png');records.push(name);}
for(const t of[0,1,2]){
 e.web_boot();run(1800);e.web_mm_view(t);page(14);shot('midi-entry-'+['light','dark','zoom'][t]);
 e.web_midi_mode(1);run(400);btn(10);shot('midi-global-'+['light','dark','zoom'][t]);btn(10);btn(13);e.web_pad(0,100);run();e.web_pad(0,0);run();e.web_mm_control(24,2,127,16,7);shot('midi-edit-'+['light','dark','zoom'][t]);
 btn(13);e.web_button(0,1);e.web_button(1,1);run(40);e.web_button(0,0);e.web_button(1,0);run(40);
 for(const p of[0,2,5]){e.web_pad(p,100);run(30);}for(const p of[0,2,5]){e.web_pad(p,0);run(30);}
 shot('midi-layer-'+['light','dark','zoom'][t]);e.web_pad(8,100);run(30);e.web_pad(8,0);run(30);shot('midi-channel-'+['light','dark','zoom'][t]);
 e.web_midi_mode(0);run(500);
}
writeFileSync('web/manual/midi-captures.json',JSON.stringify({wasm_sha256:createHash('sha256').update(readFileSync('web/mel.wasm')).digest('hex'),captures:records},null,2)+'\n');
console.log('Captured',records.length,'actual MIDI firmware screens.');
