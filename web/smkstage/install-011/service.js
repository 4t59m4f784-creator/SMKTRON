// SPDX-License-Identifier: GPL-3.0-only
// SMKSTAGE B3 alpha: independent, pinned-package install service.
import {Updater} from './protocol.js';
import {productOf, logicalImage, filesOk} from './package.js';
export const PINS=Object.freeze({
 stage:Object.freeze({name:'SMKSTAGE B3 Alpha 0.1.1-b3',product:'SMK-37 Pro_19703',size:651339,sha:'3b735c88ab65482153a903bfab9988b78b28c4f19ee064cfdb189dc57ca02c0c'}),
 stock:Object.freeze({name:'M-VAVE V016',product:'SMK-37 Pro_016',size:705252,sha:'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0'})
});
const MODEL='SMK-37 Pro';
const ALLOWED_SOURCE=Object.freeze(['SMK-37 Pro_016','SMK-37 Pro_19668','SMK-37 Pro_463','SMK-37 Pro_19703']);
export const PENDING='smkstage.b3.alpha.19703.pending.v1';
/* Rescue is permitted ONLY if this same browser retains the exact failed
 * 0.1.2 transaction: stage hash, known source ID, model and operation.
 * The 0.1.1 package remains independently SHA-256 pinned. */
const FAILED_STAGE_SHA='2d992f98864558b24d61a007e20c7c6c192a441489c5be4c90c2ca20881cd97b';
const DIAG_PENDING='smkstage.b3.diag.d1.pending.v1';
const D2_PENDING='smkstage.b3.full.d2.pending.v1';
const D3_PENDING='smkstage.b3.irq.d3.pending.v1';
const D4_PENDING='smkstage.b3.cpu0.d4.pending.v1';
const FAILED_D4_SHA='e0ef655f4f02a9c1d0d966b280e995e213a99c96afc902e9facaf43f8b10b26a';
const FAILED_D3_SHA='02504176970bc7d34b04cb0af2a23c1a56accfa4f8af67a442102e7557157405';
const FAILED_D2_SHA='4eb116f58c014ed0906ac7372b1f537bae1801cd7e4887b4925f0c51d8a3ffd2';
const FAILED_DIAG_SHA='61372501e13b6fed9fa24febad26ac4effc95266c4e2fca40b903831be3e7887';
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
  if(p&&p.model===MODEL){
   if(p.kind==='stage'&&p.sha===FAILED_STAGE_SHA&&p.source==='SMK-37 Pro_19703')
    return {model:MODEL,kind:'stage',sha:PINS.stage.sha,source:RESCUE_ID,recovery:true};
   if(p.kind==='stage'&&p.sha===PINS.stage.sha&&p.source===RESCUE_ID)
    return {model:MODEL,kind:'stage',sha:p.sha,source:p.source,recovery:true};
   if(PINS[p.kind]?.sha===p.sha&&ALLOWED_SOURCE.includes(p.source))return p;
  }
  /* D1's standalone installer leaves its own pending record. For safety,
   * permit rollback to the known-good 0.1.1 only from a known same-device
   * D1 flash attempt; never permit other identities or unpinned uploads. */
  const d=JSON.parse(storage.getItem(DIAG_PENDING));
  if(d?.model===MODEL&&d.kind==='stage'&&d.sha===FAILED_DIAG_SHA&&
     (d.source==='SMK-37 Pro_19703'||d.source===RESCUE_ID))
   return {model:MODEL,kind:'stage',sha:PINS.stage.sha,source:RESCUE_ID,recovery:true};
  const d2=JSON.parse(storage.getItem(D2_PENDING));
  if(d2?.model===MODEL&&d2.kind==='stage'&&d2.sha===FAILED_D2_SHA&&d2.source==='SMK-37 Pro_19703')
   return {model:MODEL,kind:'stage',sha:PINS.stage.sha,source:RESCUE_ID,recovery:true};
  const d3=JSON.parse(storage.getItem(D3_PENDING));
  if(d3?.model===MODEL&&d3.kind==='stage'&&d3.sha===FAILED_D3_SHA&&d3.source==='SMK-37 Pro_19703')
   return {model:MODEL,kind:'stage',sha:PINS.stage.sha,source:RESCUE_ID,recovery:true};
  const d4=JSON.parse(storage.getItem(D4_PENDING));
  if(d4?.model===MODEL&&d4.kind==='stage'&&d4.sha===FAILED_D4_SHA&&d4.source==='SMK-37 Pro_19703')
   return {model:MODEL,kind:'stage',sha:PINS.stage.sha,source:RESCUE_ID,recovery:true};
  return null;
 }catch{return null}
}
export function modeFor(kind,id,pending){
 if(!PINS[kind]||!id?.text)throw Error('Connect and identify / Conecte e identifique.');
 if(id.model!==MODEL&&id.model!=='ota-'+MODEL)throw Error('Only SMK-37 Pro / Somente SMK-37 Pro.');
 if(id.text===RESCUE_ID){
  if(kind==='stage'&&pending?.recovery===true&&pending.kind==='stage'&&pending.sha===PINS.stage.sha&&pending.source===RESCUE_ID)return 'install';
  throw Error('SMK-37 Pro USB Rescue requires a matching failed 0.1.2 transaction from this same browser. No writes sent. / Rescue exige registro da falha 0.1.2 neste navegador.');
 }
 if(isLoader(id)){
  if(pending?.model===MODEL&&pending.kind===kind&&pending.sha===PINS[kind].sha)return 'resume';
  throw Error('Update loader has no matching pending operation. Use original updater / Loader sem retomada autorizada.');
 }
 if(/^ota-/.test(id.text))throw Error('Unknown loader blocked / Loader desconhecido bloqueado.');
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
