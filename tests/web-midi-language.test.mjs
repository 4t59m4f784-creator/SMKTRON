import{test}from'node:test';import{readFileSync}from'node:fs';import assert from'node:assert/strict';
const {translate}=await import('../web/ui.js');
test('PT/EN covers MIDI pages and existing dynamic controls',()=>{
 for(const [pt,en]of[['▶ Iniciar áudio','▶ Start audio'],['Banco B','Bank B'],['K1 · PARÂMETRO 1','K1 · PARAMETER 1'],['K5 · CONTROLE','K5 · CONTROL'],['Conheça o MIDI MODE','Explore MIDI MODE'],['Manual do MIDI MODE','MIDI MODE manual'],['Guia MIDI','MIDI guide'],['Novidades da versão','What is new in this version'],['Como usar MIDI Layers','How to use MIDI Layers'],['Conheça o MIDI Layers','Explore MIDI Layers']]){assert.equal(translate(pt,'en'),en);assert.equal(translate(pt,'pt'),pt);}
 const app=readFileSync('web/app.js','utf8'),html=readFileSync('web/simulator.html','utf8');for(const id of['modeSynth','modeMidi','midiOutput','padBankMidi'])assert(html.includes(`id="${id}"`));assert(app.includes("const STORE='smktron-grain-gr3-boot-v1'"));assert(app.includes("if(m.type==='midi-out')"));assert(app.includes('old.send([0xB0|c,123,0])'));
});
