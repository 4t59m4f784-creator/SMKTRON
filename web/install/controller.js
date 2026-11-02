// SPDX-License-Identifier: GPL-3.0-only
import {InstallService,PINS,validate,modeFor,readPending} from './service.js';
const $=id=>document.getElementById(id);
let rawRc=null,rawStock=null,service=null,device=null,busy=false,choice=null;
const lines=['SMKTRON installer · nenhum dispositivo acessado ao abrir a página.'];
function log(s){lines.push(new Date().toISOString()+' '+s);$('log').textContent=lines.join('\n');$('log').scrollTop=$('log').scrollHeight}
function status(id,s){$(id).textContent=s}
function mode(kind){try{return modeFor(kind,device,readPending(localStorage))}catch{return null}}
function refresh(){
 for(const id of ['install','restore'])$(id).disabled=busy||!rawRc||!rawStock||!service||!mode(id==='install'?'rc':'stock');
 $('connect').disabled=busy;$('stockFile').disabled=busy;
 $('install').textContent=mode('rc')==='resume'?'Preparar retomada do SMKTRON':'Preparar instalação do SMKTRON';
 $('restore').textContent=mode('stock')==='resume'?'Preparar retomada do V016':'Preparar retorno ao V016';
}
async function fetchRc(){try{
 const r=await fetch(PINS.rc.url,{cache:'no-store'});if(!r.ok)throw Error('Download indisponível.');
 const raw=new Uint8Array(await r.arrayBuffer());await validate(raw,'rc');rawRc=raw;
 status('rcStatus','SMKTRON 0.3.0-gr3 verificado: modelo, SHA-256 e CRCs aprovados.');
}catch(e){rawRc=null;status('rcStatus','Pacote bloqueado: '+e.message);log('Pacote SMKTRON: '+e.message)}refresh()}
$('stockFile').addEventListener('change',async e=>{
 rawStock=null;refresh();const file=e.target.files?.[0];if(!file)return;
 status('stockStatus','Conferindo o arquivo completo…');
 try{const raw=new Uint8Array(await file.arrayBuffer());await validate(raw,'stock');rawStock=raw;status('stockStatus','V016 original confirmado. Arquivo de retorno pronto neste navegador.');log('V016 validado, SHA-256 '+PINS.stock.sha)}
 catch(error){status('stockStatus','Arquivo recusado: '+error.message);log('V016 recusado: '+error.message)}refresh();
});
$('connect').addEventListener('click',async()=>{
 if(busy)return;$('connect').disabled=true;status('deviceStatus','Solicitando acesso USB-MIDI e consultando identidade…');
 try{
  if(!navigator.requestMIDIAccess)throw Error('Use Chrome ou Edge no computador, em HTTPS.');
  const access=await navigator.requestMIDIAccess({sysex:true});
  service=new InstallService(access,localStorage,log);device=await service.identify();
  status('deviceStatus','Identidade informada: '+device.text);log('Consulta: '+device.text);
  const allowed=mode('rc')||mode('stock');status('operationStatus',allowed?'Teclado identificado. Escolha a operação e confira a confirmação.':'Identidade não habilitada para esta página; nenhuma atualização foi iniciada.');
  access.onstatechange=()=>{if(!busy){device=null;status('deviceStatus','A conexão MIDI mudou. Consulte o teclado novamente.');refresh()}};
 }catch(e){device=null;service=null;status('deviceStatus',e.message);log('Consulta falhou: '+e.message)}refresh();
});
function prepare(kind){
 if(busy||!rawRc||!rawStock||!service||!mode(kind))return;choice=kind;
 $('confirmTitle').textContent=(mode(kind)==='resume'?'Retomar: ':'')+PINS[kind].name;
 $('confirmDetails').textContent='Teclado: '+device.text+'. Destino: '+PINS[kind].product+'. Arquivo V016 de retorno conferido. Nenhum outro pacote será usado.';
 $('confirmWrite').textContent=kind==='rc'?'Confirmar SMKTRON':'Confirmar retorno ao V016';$('confirmation').showModal();
}
$('install').addEventListener('click',()=>prepare('rc'));$('restore').addEventListener('click',()=>prepare('stock'));
$('confirmation').addEventListener('close',async()=>{
 const kind=choice;choice=null;if($('confirmation').returnValue!=='confirm'||!kind||busy)return;
 busy=true;refresh();$('progress').removeAttribute('value');status('operationStatus','Conferindo novamente e iniciando a operação confirmada…');
 const run=async()=>{
  const labels={start:'Atualização confirmada',verify:'Teclado conferindo o pacote',loader:'Aguardando modo de atualização',write:'Gravando',reboot:'Aguardando reinicialização',done:'Identidade de destino confirmada'};
  const result=await service.execute(kind,rawRc,rawStock,(step,n)=>{const s=(labels[step]||step)+(n!==undefined?' · '+n:'');status('operationStatus',s);if(step!=='write'&&step!=='verify'||Number(n)%100===0)log(s)});
  device={text:result,model:'SMK-37 Pro',version:kind==='rc'?453:16};
  $('progress').value=1;status('deviceStatus','Identidade após reiniciar: '+result);
  status('operationStatus',kind==='rc'?'Concluído: o teclado informa SMK-37 Pro_453. Confira v0.3.0-gr3 na tela do instrumento.':'Concluído: o teclado informa SMK-37 Pro_016.');log('Concluído: '+result);
 };
 try{
  if(!navigator.locks?.request)throw Error('Este navegador não oferece o bloqueio necessário entre abas. Use Chrome ou Edge atualizado.');
  await navigator.locks.request('smktron-firmware-update',{ifAvailable:true},async lock=>{if(!lock)throw Error('Outra aba deste site está usando o atualizador.');await run()});
 }catch(e){device=null;$('progress').value=0;status('operationStatus','Operação não concluída: '+e.message+' Baixe o log antes de tentar outro procedimento.');log('ERRO: '+(e.stack||e.message))}
 finally{busy=false;refresh()}
});
window.addEventListener('beforeunload',e=>{if(busy){e.preventDefault();e.returnValue=''}});
$('downloadLog').addEventListener('click',()=>{const url=URL.createObjectURL(new Blob([lines.join('\n')+'\n'],{type:'text/plain'}));const a=document.createElement('a');a.href=url;a.download='smktron-ch3-installer-log.txt';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000)});
refresh();fetchRc();
