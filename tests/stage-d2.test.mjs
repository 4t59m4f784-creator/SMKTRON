import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {PINS as FULL,modeFor as fullMode,PENDING as FULL_PENDING,readPending as fullPending} from '../web/smkstage/install-d2/service.js';
import {PINS as BASE,modeFor as baseMode,readPending as basePending} from '../web/smkstage/install-011/service.js';
const id=text=>({text,model:text.startsWith('ota-')?'ota-SMK-37 Pro':'SMK-37 Pro'});
const D2_SHA='4eb116f58c014ed0906ac7372b1f537bae1801cd7e4887b4925f0c51d8a3ffd2';
const failed={model:'SMK-37 Pro',kind:'stage',sha:D2_SHA,source:'SMK-37 Pro_19703'};
const storage=p=>({getItem:k=>k===FULL_PENDING?JSON.stringify(p):null});
test('full B3 D2 exact identity/digest are pinned and immutable',()=>{
 assert.equal(FULL.stage.name,'SMKSTAGE B3 Alpha 0.1.3-d2');
 assert.equal(FULL.stage.product,'SMK-37 Pro_19703');
 assert.equal(FULL.stage.size,651339);
 assert.equal(FULL.stage.sha,D2_SHA);
 assert.equal(FULL.stock.sha,'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0');
 assert.notEqual(FULL.stage.sha,BASE.stage.sha);
});
test('D2 cannot flash from USB rescue or foreign device even with pending',()=>{
 assert.throws(()=>fullMode('stage',id('SMK-37 Pro_000'),null));
 assert.throws(()=>fullMode('stage',id('SMK-37 Pro_000'),{...failed,recovery:true}));
 assert.throws(()=>fullMode('stage',{text:'SMK-37 Elite_016',model:'SMK-37 Elite'},null));
 assert.equal(fullMode('stage',id('SMK-37 Pro_19703'),null),'install');
 assert.equal(fullPending(storage(failed))?.sha,D2_SHA);
});
test('D2 failure may only rollback to pinned 0.1.1 in same browser',()=>{
 const p=basePending(storage(failed));
 assert.equal(p?.recovery,true);
 assert.equal(p.sha,BASE.stage.sha);
 assert.equal(p.source,'SMK-37 Pro_000');
 assert.equal(baseMode('stage',id('SMK-37 Pro_000'),p),'install');
 for(const t of [{...failed,sha:'bad'},{...failed,kind:'stock'},{...failed,model:'FM-1'},{...failed,source:'SMK-37 Elite_19703'}])assert.equal(basePending(storage(t)),null);
});
test('full installer has firmware chooser, consent and functional start/restore buttons',()=>{
 const h=fs.readFileSync('web/smkstage/install-d2/index.html','utf8');
 const c=fs.readFileSync('web/smkstage/install-d2/controller.js','utf8');
 assert.match(h,/0\.1\.3-d2/);
 for(const field of ['stageFile','stockFile','connect','consent','phrase','install','restore']) assert.match(h,new RegExp('id="'+field+'"'));
 assert.match(c,/file\.arrayBuffer\(\)/);
 assert.match(c,/service\.execute\(/);
 assert.match(h,/PRESET LIBRARY/);
});
