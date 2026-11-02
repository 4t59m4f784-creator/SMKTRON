// SPDX-License-Identifier: GPL-3.0-only
import {Updater} from './protocol.js';
import {productOf,logicalImage,filesOk} from './package.js';
export const PINS=Object.freeze({
 rc:Object.freeze({name:'SMKTRON 0.3.0-ap8',product:'SMK-37 Pro_453',size:651266,sha:'7dd9edb2bb90809b8e952b463a587dbf2c26789879a74a448ad4362d0d1ab63f',url:'./firmware/smktron-0.3.0-ap8-pro.fwsc'}),
 stock:Object.freeze({name:'M-VAVE V016',product:'SMK-37 Pro_016',size:705252,sha:'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0'})
});
export const PENDING='smktron.installer.appearance.ap8.pending.v1';
export async function validate(raw,kind){
 const p=PINS[kind];if(!p)throw Error('Operação inválida.');
 if(raw.length!==p.size)throw Error('Tamanho diferente do pacote esperado.');
 const sha=[...new Uint8Array(await crypto.subtle.digest('SHA-256',raw))].map(x=>x.toString(16).padStart(2,'0')).join('');
 if(sha!==p.sha)throw Error('SHA-256 diferente. Selecione o arquivo original, sem alterações.');
 const product=productOf(raw),image=logicalImage(raw);
 if(product!==p.product||!filesOk(image))throw Error('Identidade ou CRCs inválidos.');
 return {raw,image,product,sha};
}
export function readPending(storage){try{const p=JSON.parse(storage.getItem(PENDING));return p&&PINS[p.kind]?.sha===p.sha?p:null}catch{return null}}
export function modeFor(kind,id,pending){
 const text=id?.text;if(!text)throw Error('Conecte e consulte o teclado.');
 if(id.model!=='SMK-37 Pro'&&id.model!=='ota-SMK-37 Pro')throw Error('Modelo recusado: somente SMK-37 Pro.');
 if(kind==='rc'&&['SMK-37 Pro_016','SMK-37 Pro_451','SMK-37 Pro_452','SMK-37 Pro_453','SMK-37 Pro_454'].includes(text))return 'install';
 if(kind==='stock'&&['SMK-37 Pro_451','SMK-37 Pro_452','SMK-37 Pro_453','SMK-37 Pro_454'].includes(text))return 'install';
 if(text==='ota-SMK-37 Pro_900'&&(kind==='stock'||pending?.kind===kind))return 'resume';
 if(text==='ota-SMK-37 Pro_016'&&kind==='stock'&&pending?.kind==='stock')return 'resume';
 throw Error(text==='SMK-37 Pro_016'?'O teclado já informa V016.':
  /^ota-/.test(text)?'Modo de atualização não autorizado para esta operação. Preserve o log.':
  'Firmware de origem recusado. Para instalar SMKTRON, o teclado deve informar V016 ou SMK-37 Pro_451/_452/_453/_454.');
}
function guardFor(kind,source){return (phase,id)=>{
 if(phase==='start'){
  const allowed=source();
  if(id.text!==allowed)throw Error('A identidade mudou antes da gravação. Operação bloqueada: '+id.text);
 }else if(!['ota-SMK-37 Pro_900',...(kind==='stock'?['ota-SMK-37 Pro_016']:[])].includes(id.text)){
  throw Error('Loader recusado antes da gravação: '+id.text);
 }
}}
export class InstallService{
 constructor(access,storage,log=()=>{}){this.access=access;this.storage=storage;this.log=log}
 async identify(){const up=new Updater(this.access,this.log);const d=await up.find();if(!d)throw Error('Teclado não encontrado. Confira cabo USB de dados e feche outros aplicativos MIDI.');d.link.close();return d.id}
 async execute(kind,rawRc,rawStock,step=()=>{}){
  // Recheck both buffers immediately before any MIDI query/upgrade.
  const rc=await validate(rawRc,'rc'),stock=await validate(rawStock,'stock'),pkg=kind==='rc'?rc:stock;
  if(!PINS[kind])throw Error('Operação inválida.');
  let source;
  const up=new Updater(this.access,this.log,{guard:guardFor(kind,()=>source)});
  const d=await up.find();if(!d)throw Error('Teclado não encontrado.');
  const id=d.id;source=id.text;d.link.close();const mode=modeFor(kind,id,readPending(this.storage));
  this.storage.setItem(PENDING,JSON.stringify({kind,sha:PINS[kind].sha,source:id.text}));
  if(mode==='resume'){
   const done=await up.resume(pkg.image,step,{product:pkg.product});if(!done)throw Error('Retomada não concluída. Preserve o log e consulte o teclado novamente.');
  }else await up.install(pkg.image,pkg.product,step); // strict model; anyModel stays false
  this.storage.removeItem(PENDING);return pkg.product;
 }
}
