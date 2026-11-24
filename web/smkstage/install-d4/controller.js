// SPDX-License-Identifier: GPL-3.0-only
import {InstallService,PINS,validate,modeFor,readPending} from './service.js';
const $=id=>document.getElementById(id);
let rawStage=null,rawStock=null,service=null,device=null,access=null,choice=null,busy=false;
const lines=['SMKSTAGE B3 Alpha | page opened. No MIDI connection or write request.'];
const pt=()=>document.documentElement.lang==='pt-BR';
const tr=(a,b)=>pt()?a:b;
function log(s){lines.push(new Date().toISOString()+' '+s);$('log').textContent=lines.join('\n');$('log').scrollTop=$('log').scrollHeight;}
function set(id,s,ok){const el=$(id);el.textContent=s;if(ok!==undefined)el.className='status '+(ok?'good':'bad');}
function available(kind){try{return device&&modeFor(kind,device,readPending(localStorage))}catch{return null;}}
function refresh(){
 const ready=!busy&&!!service&&!!rawStage&&!!rawStock&&$('consent').checked&&$('phrase').value.trim().toUpperCase()==='SMKSTAGE';
 $('connect').disabled=busy;$('stockFile').disabled=busy;$('stageFile').disabled=busy;
 $('install').disabled=!ready||!available('stage');$('restore').disabled=!ready||!available('stock');
 $('install').textContent=available('stage')==='resume'?tr('Retomar gravação do SMKSTAGE','Resume SMKSTAGE write'):tr('Preparar instalação experimental','Prepare experimental install');
 $('restore').textContent=available('stock')==='resume'?tr('Retomar retorno à V016','Resume V016 restore'):tr('Preparar retorno ao V016','Prepare V016 restore');
}
function applyLanguage(){
 const lang=$('language').value;document.documentElement.lang=lang==='pt'?'pt-BR':'en';
 for(const el of document.querySelectorAll('[data-pt][data-en]')){const input=el.querySelector('input');if(input){const node=[...el.childNodes].find(n=>n.nodeType===3);if(node)node.textContent=el.dataset[lang];}else el.textContent=el.dataset[lang];}
 refresh();
}
$('language').onchange=applyLanguage;
async function load(kind,file,status){
 const raw=new Uint8Array(await file.arrayBuffer());
 await validate(raw,kind);
 if(kind==='stage')rawStage=raw;else rawStock=raw;
 set(status,tr('Arquivo aprovado: identidade, SHA-256 e CRC verificados.','File approved: identity, SHA-256 and CRCs verified.'),true);
 log(PINS[kind].name+' OK ('+raw.length+' B; SHA-256 '+PINS[kind].sha+')');
}
for(const [kind,id,status] of [['stage','stageFile','stageStatus'],['stock','stockFile','stockStatus']]){
 $(id).addEventListener('change',async(e)=>{
  if(kind==='stage')rawStage=null;else rawStock=null;
  const file=e.target.files?.[0];if(!file){refresh();return;}
  set(status,tr('Conferindo pacote…','Checking package…'));
  try{await load(kind,file,status)}catch(err){set(status,tr('Arquivo recusado: ','File refused: ')+err.message,false);log(err.message)}
  refresh();
 });
}
$('connect').onclick=async()=>{
 if(busy)return;service=device=null;$('connect').disabled=true;
 set('deviceStatus',tr('Solicitando USB-MIDI e consultando identidade…','Requesting USB-MIDI and reading identity…'));
 try{
  if(!navigator.requestMIDIAccess)throw Error(tr('Use Chrome/Edge no computador com HTTPS.','Use Chrome/Edge desktop with HTTPS.'));
  access=await navigator.requestMIDIAccess({sysex:true});service=new InstallService(access,localStorage,log);
  device=await service.identify();log('Device '+device.text);
  let compatible=false;for(const kind of ['stage','stock']){if(available(kind)){compatible=true;break}}
  if(!compatible){const cause=(()=>{try{return modeFor('stage',device,readPending(localStorage))}catch(e){return e.message}})();throw Error(cause);}
  set('deviceStatus',tr('Identidade confirmada: ','Identity confirmed: ')+device.text,true);
  set('operationStatus',tr('Consulta concluída, sem alterações no teclado. Verifique arquivos, consentimento e operação.','Read-only query completed. Check files, consent and operation.'),true);
  access.onstatechange=()=>{if(!busy){device=service=null;set('deviceStatus',tr('Conexão mudou. Consulte novamente.','Connection changed. Please identify again.'),false);refresh();}};
 }catch(e){device=service=null;set('deviceStatus',e.message,false);log('Identify: '+e.message)}
 refresh();
};
$('consent').onchange=refresh;$('phrase').oninput=refresh;
function prepare(kind){
 if(busy||$(kind==='stage'?'install':'restore').disabled||!available(kind))return;
 choice=kind;const p=PINS[kind];
 $('confirmTitle').textContent=kind==='stage'?tr('Confirmar firmware B3 experimental','Confirm experimental B3 firmware'):tr('Confirmar retorno ao V016','Confirm return to V016');
 $('confirmDetails').textContent=tr('Dispositivo: ','Device: ')+device.text+tr('. Destino: ','. Target: ')+p.product+tr('. Arquivos validados. Risco de perda de dados e falha na inicialização.','. Validated files. Risk of data loss and failure to boot.');
 $('confirmation').returnValue='cancel';$('confirmation').showModal();
}
$('install').onclick=()=>prepare('stage');$('restore').onclick=()=>prepare('stock');
$('confirmation').addEventListener('close',async()=>{
 const kind=choice;choice=null;
 if($('confirmation').returnValue!=='confirm'||!kind||busy||!available(kind))return;
 busy=true;refresh();$('progress').removeAttribute('value');
 set('operationStatus',tr('Iniciando somente a operação confirmada…','Starting only the confirmed operation…'));
 try{
  if(!navigator.locks?.request)throw Error(tr('Navegador sem trava entre abas. Use Chrome/Edge atualizado.','Browser lacks tab locks. Use recent Chrome/Edge.'));
  await navigator.locks.request('smk37-pro-firmware-update',{ifAvailable:true},async lock=>{
   if(!lock)throw Error(tr('Outra aba já utiliza este instalador.','Another tab is using this installer.'));
   const phases={start:tr('Identidade conferida','Identity verified'),verify:tr('Verificando pacote','Verifying package'),loader:tr('Aguardando loader','Waiting for loader'),write:tr('Gravando setores','Writing sectors'),reboot:tr('Aguardando reinicialização','Waiting for reboot'),done:tr('Destino confirmado','Target confirmed')};
   const result=await service.execute(kind,rawStage,rawStock,(step,n)=>{
    const s=(phases[step]||step)+(n===undefined?'':' · '+n);
    set('operationStatus',s);
    if(step!=='write'&&step!=='verify'||Number(n)%100===0)log(s);
   });
   device={text:result,model:'SMK-37 Pro'};
   $('progress').value=1;
   set('deviceStatus',tr('Após reiniciar: ','After reboot: ')+result,true);
   set('operationStatus',tr('Gravação concluída e identidade confirmada.','Write completed and target identity confirmed.'),true);
   log('Completed: '+result);
  });
 }catch(e){device=service=null;$('progress').value=0;set('operationStatus',tr('Não concluído. Preserve o registro: ','Not completed. Keep the log: ')+e.message,false);log('ERROR '+(e.stack||e.message))}
 finally{busy=false;refresh();}
});
$('downloadLog').onclick=()=>{
 const url=URL.createObjectURL(new Blob([lines.join('\n')+'\n'],{type:'text/plain'})),a=document.createElement('a');
 a.href=url;a.download='SMKSTAGE-B3-alpha-install-log.txt';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
};
window.addEventListener('beforeunload',e=>{if(busy){e.preventDefault();e.returnValue='';}});
applyLanguage();refresh();
