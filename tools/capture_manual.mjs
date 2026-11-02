// Capture real firmware framebuffers; no redraws of the interface.
import{readFileSync,writeFileSync,mkdirSync}from'node:fs';import{createHash}from'node:crypto';
const wasm=readFileSync('dist/chords-preview/mel.wasm');mkdirSync('manual/capture-work',{recursive:true});
const run=(e,ms)=>{for(let i=0;i<Math.ceil(ms*44.1/128);i++)e.web_render(128);};
const tap=(e,id)=>{e.web_button(id,1);run(e,30);e.web_button(id,0);run(e,60);};
const pad=(e,p,ms=40)=>{e.web_pad(p,100);run(e,ms);e.web_pad(p,0);run(e,80);};
const keys=(e,ns,down)=>{for(const n of ns)e.web_key(n,down?100:0);run(e,100);};
function pg(e,p){for(let i=0;e.web_probe(0)!==p&&i<20;i++)tap(e,3);if(e.web_probe(0)!==p)throw Error('page '+p);}
function enc(e,id,n){e.web_enc(id,n);run(e,100);}
const names=['play','layer','tape','wear','vibrato','leslie','echo','reverb','eq','setup','arp','seq','system','chords'];
const manifest={wasm_sha256:createHash('sha256').update(wasm).digest('hex'),width:240,height:240,captures:[],pads:{}};
for(const theme of ['light','dark'])for(let page=0;page<15;page++){
 const e=(await WebAssembly.instantiate(wasm,{})).instance.exports;e.web_boot();run(e,1700);
 if(theme==='dark'){pg(e,12);enc(e,1,1);}
 const name=page===14?'presets':names[page];pg(e,page===14?0:page);
 if(page===1)enc(e,0,2); // CHOIR layer, 40%
 if([4,5,6].includes(page))enc(e,3,25);
 if(page===10){tap(e,6);keys(e,[12,16,19],true);}
 else if(page===11){enc(e,0,-12);tap(e,17);for(const ns of [[12,16,19],[9,12,16],[5,9,12],[7,11,14]]){keys(e,ns,true);keys(e,ns,false);}tap(e,17);tap(e,16);}
 else if(page===13){e.web_chord_mode(1);run(e,50);for(const [i,ns]of[[0,[12,16,19]],[1,[9,12,16]],[2,[5,9,12]],[3,[7,11,14]]]){pad(e,8);pad(e,i);keys(e,ns,true);keys(e,ns,false);}e.web_chord_mixer(0,40);e.web_chord_mixer(1,-20);e.web_chord_mixer(2,30);pad(e,9);pad(e,0);keys(e,[24],true);}
 else if(page===14){e.web_button(11,1);run(e,700);for(let i=0;i<4;i++){enc(e,7,i?1:0);pad(e,i,1100);}pad(e,8);e.web_button(11,0);run(e,50);pad(e,1);}
 else keys(e,[12,16,19],true);
 run(e,1650); // let transient messages clear; live reels/keys remain real.
 const filename=name+'-'+theme;writeFileSync('manual/capture-work/'+filename+'.rgb565',new Uint8Array(e.memory.buffer,e.web_screen(),115200));
 manifest.captures.push({name,theme,file:'images/'+filename+'.png',page:e.web_probe(0),params:Array.from({length:4},(_,i)=>e.web_page_param(e.web_probe(0),i))});
 if(theme==='light'&&[0,13,14].includes(page))manifest.pads[page===0?'normal':page===13?'chords':'presets']=Array.from({length:16},(_,i)=>{const c=e.web_pad_color(i);return[(c>>8)&255,(c>>16)&255,c&255];});
}
writeFileSync('dist/manual/captures.json',JSON.stringify(manifest,null,2)+'\n');
