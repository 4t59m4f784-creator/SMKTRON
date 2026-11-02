import{readFileSync,writeFileSync}from'node:fs';
const run=(e,ms)=>{for(let i=0;i<ms*44.1/128;i++)e.web_render(128)};
const btn=(e,k)=>{e.web_button(k,1);run(e,40);e.web_button(k,0);run(e,40);};
const pad=(e,k,ms=40)=>{e.web_pad(k,100);run(e,ms);e.web_pad(k,0);run(e,40);};
const rows=[];
for(const [version,path]of[['AT1','/tmp/smktron-at1.wasm'],['AT2','dist/audio-preview/mel.wasm']])for(let sound=1;sound<4;sound++){
 const e=(await WebAssembly.instantiate(readFileSync(path),{})).instance.exports;e.web_boot();run(e,1700);btn(e,0);pad(e,0,1100);e.web_enc(7,sound);run(e,60);pad(e,1,1100);pad(e,0);for(const k of[12,16,19])e.web_key(k,100);run(e,1500);e.web_pad(1,100);run(e,40);e.web_pad(1,0);
 let last=0,maxStep=0,ss=0;for(let b=0;b<50;b++){e.web_render(128);const a=new Float32Array(e.memory.buffer,e.web_out_l(),128);for(const x of a){if(b||ss)maxStep=Math.max(maxStep,Math.abs(x-last));last=x;ss+=x*x;}}
 rows.push({version,to:['FLUTE','CHOIR','CELLO','VIOLINS'][sound],maxStep,energy:ss});
}
writeFileSync('experiments/audio-audit/scene-measure.json',JSON.stringify(rows,null,2));console.log(rows);
