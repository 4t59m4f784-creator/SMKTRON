// SPDX-License-Identifier: GPL-3.0-only
// SMKSTAGE B3 alpha: independent, pinned-package install service.
import {Updater} from './protocol.js';
import {productOf, logicalImage, filesOk} from './package.js';
export const PINS=Object.freeze({
 stage:Object.freeze({name:'SMKSTAGE B3 Alpha 0.1.3-d2',product:'SMK-37 Pro_19703',size:651339,sha:'4eb116f58c014ed0906ac7372b1f537bae1801cd7e4887b4925f0c51d8a3ffd2'}),
 stock:Object.freeze({name:'M-VAVE V016',product:'SMK-37 Pro_016',size:705252,sha:'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0'})
});
const MODEL='SMK-37 Pro';
const ALLOWED_SOURCE=Object.freeze(['SMK-37 Pro_016','SMK-37 Pro_19668','SMK-37 Pro_463','SMK-37 Pro_19703']);
export const PENDING='smkstage.b3.full.d2.pending.v1';
/* Rescue is permitted ONLY if this same browser retains the exact failed
 * 0.1.2 transaction: stage hash, known source ID, model and operation.
 * The 0.1.1 package remains independently SHA-256 pinned. */
const FAILED_STAGE_SHA='2d992f98864558b24d61a007e20c7c6c192a441489c5be4c90c2ca20881cd97b';
const RESCUE_ID='SMK-37 Pro_000';
const isLoader=(id)=>id?.text==='ota-SMK-37 Pro_900';
export async function validate(raw,kind){
 const p=PINS[kind];if(!p)throw Error('Unknown operation / Operação desconhecida.');
 if(!(raw instanceof Uint8Array)||raw.length!==p.size)throw Error('Wrong package length / Tamanho de arquivo incorreto.');
 if(!globalThis.crypto?.subtle)throw Error('HTTPS and secure browser required.');
 const digest=Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',raw)),v=>v.toString(16).padStart(2,'0')).join('');
 if(digest!==p.sha)throw Error('SHA-256 differs from approved original / SHA-256 inválido.');
 const product=productOf(raw),image=logicalImage(raw);
 if(product!==p.product||!filesOk(image))throw Error('Invalid firmware identity or CRC / Identidade ou CRC inválidos.');
 return {product,image,sha:digest,raw};
}
export function readPending(storage){
 try{
  const p=JSON.parse(storage.getItem(PENDING));
  return p&&p.model===MODEL&&PINS[p.kind]?.sha===p.sha&&ALLOWED_SOURCE.includes(p.source)?p:null;
 }catch{return null}
}
export function modeFor(kind,id,pending){
 if(!PINS[kind]||!id?.text)throw Error('Connect and identify / Conecte e identifique.');
 if(id.model!==MODEL&&id.model!=='ota-'+MODEL)throw Error('Only SMK-37 Pro / Somente SMK-37 Pro.');
 if(id.text===RESCUE_ID)throw Error('Use the pinned 0.1.1 fallback installer for USB Rescue / Use o instalador de retorno 0.1.1 em Rescue.');
 if(id.text===RESCUE_ID){
  if(kind==='stage'&&pending?.recovery===true&&pending.kind==='stage'&&pending.sha===PINS.stage.sha&&pending.source===RESCUE_ID)return 'install';
  throw Error('SMK-37 Pro USB Rescue requires a matching failed 0.1.2 transaction from this same browser. No writes sent. / Rescue exige registro da falha 0.1.2 neste navegador.');
 }
 if(isLoader(id)){
  if(pending?.model===MODEL&&pending.kind===kind&&pending.sha===PINS[kind].sha)return 'resume';
  throw Error('Update loader has no matching pending operation. Use original updater / Loader sem retomada autorizada.');
 }
 if(/^ota-/.test(id.text))throw Error('Unknown loader blocked / Loader desconhecido bloqueado.');
 if(id.text==='SMK-37 Pro_000')throw Error('Use the 0.1.1 fallback installer to recover from USB Rescue; never loop a diagnostic build.');
 if(!ALLOWED_SOURCE.includes(id.text))throw Error('Firmware origin not approved for this alpha / Origem não autorizada: '+id.text);
 if(kind==='stock'&&id.text==='SMK-37 Pro_016')throw Error('Already factory V016 / Já está na V016.');
 return 'install';
}
function guardFor(source){
 return (phase,id)=>{
  if(phase==='start'){
   if(id.text!==source)throw Error('Device identity changed before transfer / Identidade mudou antes da transferência.');
  }else if(!isLoader(id))throw Error('Different updater loader; refused before writing / Loader diferente; gravação bloqueada: '+id.text);
 };
}
export class InstallService{
 constructor(access,storage,log=()=>{}){Object.assign(this,{access,storage,log})}
 async identify(){
  const u=new Updater(this.access,this.log);const d=await u.find();
  if(!d)throw Error('SMK-37 Pro not found / Teclado não encontrado.');
  d.link.close();return d.id;
 }
 async execute(kind,stageRaw,stockRaw,step=()=>{}){
  if(!PINS[kind])throw Error('Invalid operation.');
  // Both files revalidated immediately before querying updater; no network firmware fetches.
  const stage=await validate(stageRaw,'stage'),stock=await validate(stockRaw,'stock');
  const pkg=kind==='stage'?stage:stock;
  const u=new Updater(this.access,this.log);
  const d=await u.find();if(!d)throw Error('Keyboard not connected.');
  const id=d.id;d.link.close();
  const resume=modeFor(kind,id,readPending(this.storage));
  const source=resume==='resume'?readPending(this.storage).source:id.text;
  u.guard=guardFor(resume==='resume'?id.text:source);
  this.storage.setItem(PENDING,JSON.stringify({model:MODEL,kind,sha:PINS[kind].sha,source}));
  try{
   if(resume==='resume'){
    if(!await u.resume(pkg.image,step,{product:pkg.product}))throw Error('Resume incomplete / Retomada incompleta.');
   }else await u.install(pkg.image,pkg.product,step);
  }catch(e){throw e}
  this.storage.removeItem(PENDING);
  return pkg.product;
 }
}
