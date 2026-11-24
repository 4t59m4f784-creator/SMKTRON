import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {PINS,modeFor,readPending,validate,PENDING} from '../web/smkstage/install/service.js';
const ID=text=>({text,model:text.startsWith('ota-')?'ota-SMK-37 Pro':'SMK-37 Pro'});
const empty={getItem(){return null}};
test('B3 and stock pins are immutable and distinct',()=>{
 assert.equal(PINS.stage.product,'SMK-37 Pro_19703');
 assert.equal(PINS.stage.size,651339);
 assert.equal(PINS.stage.sha,'2d992f98864558b24d61a007e20c7c6c192a441489c5be4c90c2ca20881cd97b');
 assert.equal(PINS.stock.product,'SMK-37 Pro_016');
 assert.equal(PINS.stock.size,705252);
});
test('known firmware origins only',()=>{
 for(const n of ['016','19668','463','19703'])assert.throws(()=>modeFor('stage',ID('SMK-37 Pro_'+n),null),/withdrawn/);
 for(const n of ['19668','463','19703'])assert.equal(modeFor('stock',ID('SMK-37 Pro_'+n),null),'install');
 assert.throws(()=>modeFor('stock',ID('SMK-37 Pro_016'),null));
 for(const n of ['19667','19664','462','19702','451','999'])assert.throws(()=>modeFor('stage',ID('SMK-37 Pro_'+n),null));
 assert.throws(()=>modeFor('stage',{text:'SMK-37 Elite_016',model:'SMK-37 Elite'},null));
 assert.throws(()=>modeFor('stage',{text:'FM-1_900',model:'FM-1'},null));
});
test('loader never authorized without exact pending operation',()=>{
 assert.throws(()=>modeFor('stage',ID('ota-SMK-37 Pro_900'),null));
 assert.throws(()=>modeFor('stage',ID('ota-SMK-37 Pro_901'),null));
 const pending={kind:'stage',sha:PINS.stage.sha,source:'SMK-37 Pro_19668',model:'SMK-37 Pro'};
 assert.throws(()=>modeFor('stage',ID('ota-SMK-37 Pro_900'),pending),/withdrawn/);
 assert.throws(()=>modeFor('stock',ID('ota-SMK-37 Pro_900'),pending));
 const storage={getItem:key=>key===PENDING?JSON.stringify(pending):null};
 assert.deepEqual(readPending(storage),pending);
 const invalid={...pending,source:'SMK-37 Pro_999'};
 assert.equal(readPending({getItem:()=>JSON.stringify(invalid)}),null);
});
test('unapproved and corrupt packages refused',async()=>{
 await assert.rejects(validate(new Uint8Array(651338),'stage'));
 await assert.rejects(validate(new Uint8Array(651339),'stage'));
});
test('web markup contains expected wired safety controls',()=>{
 const root='web/smkstage/install/';
 const html=fs.readFileSync(root+'index.html','utf8');
 const ctrl=fs.readFileSync(root+'controller.js','utf8');
 for(const id of ['stageFile','stockFile','connect','consent','phrase','install','restore','confirmation','progress','log'])assert.match(html,new RegExp('id="'+id+'"'));
 assert.match(ctrl,/file\.arrayBuffer\(\)/);
 assert.match(ctrl,/service\.identify\(\)/);
 assert.match(ctrl,/service\.execute\(/);
 assert.match(ctrl,/returnValue!=='confirm'/);
 assert.match(ctrl,/navigator\.locks\.request/);
});
