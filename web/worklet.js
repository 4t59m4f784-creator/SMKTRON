// SPDX-License-Identifier: GPL-3.0-only
// Adapted from FMEL-1 web/emu/worklet.js (see LICENSING.md).
class Smktron extends AudioWorkletProcessor {
 constructor(){super();this.ex=null;this.last=0;this.was=0;this.sceneWas=0;this.chordWas=0;this.port.onmessage=e=>this.message(e.data)}
 async message(m){
  if(m.type==='load'){
   try{const {instance}=await WebAssembly.instantiate(m.wasm,{});this.ex=instance.exports;
    if(m.settings?.length&&m.settings.length<=256){new Uint8Array(this.ex.memory.buffer,this.ex.web_settings(),m.settings.length).set(m.settings);this.ex.web_settings_set_len(m.settings.length)}
    if(m.sceneStore?.length===8192)new Uint8Array(this.ex.memory.buffer,this.ex.web_scene_flash(),8192).set(m.sceneStore);
    if(m.chordStore?.length===8192)new Uint8Array(this.ex.memory.buffer,this.ex.web_chord_flash(),8192).set(m.chordStore);
    this.ex.web_master(700);this.ex.web_boot();this.was=this.ex.web_settings_writes();this.port.postMessage({type:'ready',master:this.ex.web_probe(17),fresh:!m.settings?.length});
   }catch(e){this.port.postMessage({type:'error',message:String(e)});this.ex=null}return;
  }
  const e=this.ex;if(!e)return;
  switch(m.type){
   case 'battery-test':e.web_battery_test(m.low?520:650,0);break;
   case 'grain':e.web_grain_set(m.k,m.value);break;
   case 'grain-style':e.web_grain_style(m.value);break;
   case 'grain-on':e.web_grain_on(m.value?1:0);break;
   case 'key':e.web_key(m.k,m.vel);break;
   case 'pad':e.web_pad(m.k,m.vel);break;
   case 'pad-cancel':if(e.web_chord_state(0))e.web_pad(m.k,0);else e.web_scene_cancel_pad(m.k);break;
   case 'save-cancel':e.web_scene_cancel_save();break;
   case 'button':e.web_button(m.id,m.down?1:0);break;
   case 'enc':e.web_enc(m.id,m.n);break;
   case 'master':e.web_smk_master(m.value);break;
   case 'bypass':{for(const [id,probe] of [[7,20],[14,29]])if(!!e.web_probe(probe)!==!!m.value)e.web_button(id,1);const end=e.web_now_ms()+700;while(e.web_now_ms()<end)e.web_render(128);e.web_button(7,0);e.web_button(14,0);break;}
   case 'fader':e.web_fader(m.id,m.value);break;
   case 'wheel':e.web_wheel(m.id,m.value);break;
   case 'midi':e.web_midi(m.data[0]||0,m.data[1]||0,m.data[2]||0);break;
   case 'panic':e.web_panic();break;
  }
 }
 process(_,outputs){
  const e=this.ex,out=outputs[0];if(!e)return true;
  try{
   e.web_render(out[0].length);out[0].set(new Float32Array(e.memory.buffer,e.web_out_l(),out[0].length));
   if(out[1])out[1].set(new Float32Array(e.memory.buffer,e.web_out_r(),out[1].length));
   if(currentTime-this.last>1/25){this.last=currentTime;
    const fb=new Uint16Array(e.memory.buffer,e.web_screen(),240*240).slice();
    const msg={type:'frame',fb,pads:Array.from({length:16},(_,i)=>e.web_pad_color(i)),state:Array.from({length:30},(_,i)=>e.web_probe(i))};
    msg.grain=[e.web_grain_state(0),e.web_grain_state(1)];msg.bank=e.web_fader_bank();msg.scenes=[e.web_scene_mode(),e.web_scene_pin(),e.web_scene_mask(),e.web_scene_selected(),e.web_scene_loaded()];
    msg.chords={state:Array.from({length:10},(_,i)=>e.web_chord_state(i)),bank:Array.from({length:8},(_,p)=>Array.from({length:6},(_,i)=>e.web_chord_note(p,i)).filter(n=>n>=0))};
    if(this.chordWas!==e.web_chord_writes()){this.chordWas=e.web_chord_writes();msg.chordStore=Array.from(new Uint8Array(e.memory.buffer,e.web_chord_flash(),8192))}
    if(this.sceneWas!==e.web_scene_writes()){this.sceneWas=e.web_scene_writes();msg.sceneStore=Array.from(new Uint8Array(e.memory.buffer,e.web_scene_flash(),8192))}
    if(this.was!==e.web_settings_writes()){this.was=e.web_settings_writes();const n=e.web_settings_len();if(n>=0)msg.settings=Array.from(new Uint8Array(e.memory.buffer,e.web_settings(),n))}
    this.port.postMessage(msg,[fb.buffer]);
   }
  }catch(err){this.port.postMessage({type:'error',message:String(err)});this.ex=null}
  return true;
 }
}
registerProcessor('smktron',Smktron);
