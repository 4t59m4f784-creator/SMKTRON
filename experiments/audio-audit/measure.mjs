import{readFileSync,writeFileSync}from'node:fs';
const wasm=readFileSync('dist/pads-preview/mel.wasm');
const run=(e,ms)=>{for(let i=0;i<ms*44.1/128;i++)e.web_render(128)};
function page(e,p){for(let i=0;i<20&&e.web_probe(0)!==p;i++){e.web_button(3,1);run(e,30);e.web_button(3,0);run(e,30);}}
const results=[];
for(let tape=0;tape<4;tape++)for(const reverb of[0,19,50,80]){
 const e=(await WebAssembly.instantiate(wasm,{})).instance.exports;e.web_boot();run(e,1700);e.web_enc(7,tape);run(e,100);page(e,7);e.web_enc(3,reverb-19);run(e,100);
 for(const k of[12,16,19,24,28,31])e.web_key(k,100);
 let peak=0,clipped=0,delta=0,last=[0,0],ss=0,n=0;
 for(let b=0;b<Math.ceil(16*44100/128);b++){e.web_render(128);for(let c=0;c<2;c++){for(const v of new Float32Array(e.memory.buffer,e[c?'web_out_r':'web_out_l'](),128)){peak=Math.max(peak,Math.abs(v));if(Math.abs(v)>=32767/32768)clipped++;delta=Math.max(delta,Math.abs(v-last[c]));last[c]=v;ss+=v*v;n++;}}}
 results.push({tape:['FLUTE','CHOIR','CELLO','VIOLINS'][tape],reverb,peak,clipSamples:clipped,maxStep:delta,rms:Math.sqrt(ss/n)});
}
writeFileSync('experiments/audio-audit/baseline.json',JSON.stringify(results,null,2));console.log(results);
