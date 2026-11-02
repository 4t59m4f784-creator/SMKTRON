// SPDX-License-Identifier: GPL-3.0-only
import {LabEngine} from './lab-engine.js?v=ap8';
class Smktron extends AudioWorkletProcessor{
 constructor(){super();this.lab=new LabEngine();this.last=0;this.loading=false;this.failed=false;this.port.onmessage=e=>this.message(e.data);}
 async message(m){
  try{
   if(m.type==='load-pair'){this.loading=true;await this.lab.loadPair(m.builds);this.failed=false;this.port.postMessage({type:'ready'});this.loading=false;return;}
   if(!this.loading)this.lab.control(m);
  }catch(e){this.loading=false;this.port.postMessage({type:'error',message:String(e.message||e)});}
 }
 process(_,outputs){
  if(this.loading||this.failed)return true;
  try{
   const audio=this.lab.render(outputs[0][0].length);if(audio){outputs[0][0].set(audio[0]);if(outputs[0][1])outputs[0][1].set(audio[1]);}
   if(currentTime-this.last>1/25){this.last=currentTime;const m=this.lab.frame();if(m)this.port.postMessage(m,[m.fb.buffer]);}
  }catch(e){this.failed=true;this.port.postMessage({type:'error',fatal:true,message:String(e.message||e)});}
  return true;
 }
}
registerProcessor('smktron',Smktron);
