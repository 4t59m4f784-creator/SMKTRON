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
mkdirSync('web/images',{recursive:true});e.web_boot();run(3500);page(12);e.web_enc(1,1);run();page(0);for(const k of[12,16,19])e.web_key(k,100);run(300);capture('patch');
page(1);e.web_enc(0,4);run(300);assert.equal(e.web_page_param(1,0),4);capture('layer');console.log('Captured actual Dark PATCH and LAYER screens.');
