import{readFileSync,writeFileSync}from'node:fs';
const run=(e,ms)=>{for(let i=0;i<ms*44.1/128;i++)e.web_render(128)};
function pg(e,p){for(let i=0;i<20&&e.web_probe(0)!==p;i++){e.web_button(3,1);run(e,30);e.web_button(3,0);run(e,30);}}
async function prepare(path,type){const e=(await WebAssembly.instantiate(readFileSync(path),{})).instance.exports;e.web_boot();run(e,1700);pg(e,type==='vibrato'?4:7);if(type==='vibrato')e.web_enc(3,40);if(type==='reverb')e.web_enc(3,50);run(e,100);for(const k of[12,16,19])e.web_key(k,100);run(e,2800);return e;}
const results=[];
for(const [version,path]of[['PM1','dist/pads-preview/mel.wasm'],['AT1','dist/audio-preview/mel.wasm']])for(const type of['reverb','vibrato','fader','scene-volume','master-volume']){
 const a=await prepare(path,type),b=await prepare(path,type);
 if(type==='reverb'||type==='vibrato')a.web_enc(3,-100);
 if(type==='fader')a.web_fader(3,127);
 if(type==='scene-volume')a.web_smk_master(400);
 if(type==='master-volume'){pg(a,12);pg(b,12);a.web_enc(2,-40);}
 let last=0,step=0,energy=0,peak=0,first=null;
 for(let i=0;i<24;i++){a.web_render(128);b.web_render(128);const aa=new Float32Array(a.memory.buffer,a.web_out_l(),128),bb=new Float32Array(b.memory.buffer,b.web_out_l(),128);for(let j=0;j<128;j++){const d=aa[j]-bb[j],dd=d-last;if(first===null&&d!==0)first=i*128+j;step=Math.max(step,Math.abs(dd));energy+=dd*dd;peak=Math.max(peak,Math.abs(aa[j]));last=d;}}
 results.push({version,type,maxResidualStep:step,residualDerivativeEnergy:energy,firstChangedSample:first,peak});
}
writeFileSync('experiments/audio-audit/transitions.json',JSON.stringify(results,null,2));console.log(results);
