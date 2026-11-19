// SPDX-License-Identifier: GPL-3.0-only
import {Updater} from '../install/protocol.js';
import {productOf,logicalImage,filesOk} from '../install/package.js';
export const PINS=Object.freeze({
 rc:Object.freeze({name:'SMKTRON 0.3.0-elite1',product:'SMK-37 Elite_463',size:651270,sha:'4e78c864ea788298c81b14ee6ed68d70b2d3249cc7094b91191b59201b9684f4',url:'./firmware/smktron-0.3.0-elite1.fwsc'}),
 stock:Object.freeze({name:'M-VAVE V016',product:'SMK-37 Elite_016',size:705236,sha:'b2ee38abfef3ae922e4eecdb8a551e14345300ae6f270265feacd33b6ef99cb6'})
});
export const PENDING='smktron.installer.elite1.pending.v1';
export async function validate(raw,kind){
 const p=PINS[kind];if(!p)throw Error('Operação inválida.');
 if(raw.length!==p.size)throw Error('Tamanho diferente do pacote esperado.');
 const sha=[...new Uint8Array(await crypto.subtle.digest('SHA-256',raw))].map(x=>x.toString(16).padStart(2,'0')).join('');
 if(sha!==p.sha)throw Error('SHA-256 diferente. Selecione o arquivo original, sem alterações.');
 const product=productOf(raw),image=logicalImage(raw);
 if(product!==p.product||!filesOk(image))throw Error('Identidade ou CRCs inválidos.');
 return {raw,image,product,sha};
}
const originAllowed=(kind,text)=>kind==='rc'?['SMK-37 Elite_016','SMK-37 Elite_463'].includes(text):kind==='stock'&&['SMK-37 Elite_463','SMK-37 Elite_000'].includes(text);
export function readPending(storage){try{const p=JSON.parse(storage.getItem(PENDING));return p&&PINS[p.kind]?.sha===p.sha&&originAllowed(p.kind,p.source)?p:null}catch{return null}}
export function modeFor(kind,id,pending){
 if(!PINS[kind]||!id?.text)throw Error('Conecte e consulte o teclado.');
 if(!['SMK-37 Elite','ota-SMK-37 Elite'].includes(id.model))throw Error('Modelo recusado: somente SMK-37 Elite.');
 if(originAllowed(kind,id.text))return 'install';
 const loaders=kind==='stock'?['ota-SMK-37 Elite_900','ota-SMK-37 Elite_016']:['ota-SMK-37 Elite_900'];
 if(loaders.includes(id.text)&&pending?.kind===kind&&pending.sha===PINS[kind].sha&&originAllowed(kind,pending.source))return 'resume';
 throw Error('Origem recusada. Use V016 oficial ou uma versão SMKTRON reconhecida pelo instalador.');
}
function guardFor(kind,source){return (phase,id)=>{
 if(phase==='start'){
  const allowed=source();
  if(id.text!==allowed)throw Error('A identidade mudou antes da gravação. Operação bloqueada: '+id.text);
 }else if(!['ota-SMK-37 Elite_900',...(kind==='stock'?['ota-SMK-37 Elite_016']:[])].includes(id.text)){
  throw Error('Loader recusado antes da gravação: '+id.text);
 }
}}
export class InstallService{
 constructor(access,storage,log=()=>{},UpdaterType=Updater){this.access=access;this.storage=storage;this.log=log;this.UpdaterType=UpdaterType}
 async identify(){const up=new this.UpdaterType(this.access,this.log);const d=await up.find();if(!d)throw Error('Teclado não encontrado. Confira cabo USB de dados e feche outros aplicativos MIDI.');d.link.close();return d.id}
 async execute(kind,rawRc,rawStock,step=()=>{}){
  // Recheck both buffers immediately before any MIDI query/upgrade.
  const rc=await validate(rawRc,'rc'),stock=await validate(rawStock,'stock'),pkg=kind==='rc'?rc:stock;
  if(!PINS[kind])throw Error('Operação inválida.');
  let source;
  const up=new this.UpdaterType(this.access,this.log,{guard:guardFor(kind,()=>source)});
  const d=await up.find();if(!d)throw Error('Teclado não encontrado.');
  const id=d.id;source=id.text;d.link.close();const pending=readPending(this.storage),mode=modeFor(kind,id,pending),origin=mode==='resume'?pending.source:id.text;
  this.storage.setItem(PENDING,JSON.stringify({kind,sha:PINS[kind].sha,source:origin}));
  if(mode==='resume'){
   const done=await up.resume(pkg.image,step,{product:pkg.product});if(!done)throw Error('Retomada não concluída. Preserve o log e consulte o teclado novamente.');
  }else await up.install(pkg.image,pkg.product,step); // strict model; anyModel stays false
  this.storage.removeItem(PENDING);return pkg.product;
 }
}
