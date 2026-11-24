import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {PINS as DP,modeFor as diagMode,PENDING as DPENDING} from '../web/smkstage/install-d1/service.js';
import {PINS as FP,modeFor as fallbackMode,readPending as fallbackPending} from '../web/smkstage/install-011/service.js';
const id=text=>({text,model:text.startsWith('ota-')?'ota-SMK-37 Pro':'SMK-37 Pro'});
const D='61372501e13b6fed9fa24febad26ac4effc95266c4e2fca40b903831be3e7887';
const B='3b735c88ab65482153a903bfab9988b78b28c4ba7b907c5489afa1c96b5d4862ec5db68';
const flashAttempt={model:'SMK-37 Pro',kind:'stage',sha:D,source:'SMK-37 Pro_19703'};
const local=d=>({getItem:k=>k===DPENDING?JSON.stringify(d):null});
test('diagnostic D1 is distinct from baseline and pinned to exact bytes',()=>{
 assert.equal(DP.stage.name,'SMKSTAGE B3 Alpha 0.1.1-d1');
 assert.equal(DP.stage.product,'SMK-37 Pro_19703');
 assert.equal(DP.stage.size,651339);
 assert.equal(DP.stage.sha,D);
 assert.notEqual(DP.stage.sha,FP.stage.sha);
 assert.equal(FP.stage.sha,'3b735c88ab65482153a903bfab9988b78b28c4f19ee064cfdb189dc57ca02c0c');
});
test('diag refuses USB Rescue, foreign model and unknown loader',()=>{
 assert.throws(()=>diagMode('stage',id('SMK-37 Pro_000'),flashAttempt));
 assert.throws(()=>diagMode('stage',id('ota-SMK-37 Pro_901'),null));
 assert.throws(()=>diagMode('stage',{text:'FM-1_000',model:'FM-1'},null));
 assert.equal(diagMode('stage',id('SMK-37 Pro_19703'),null),'install');
});
test('failed D1 enables only 0.1.1 fallback and keeps original stock pin',()=>{
 const pending=fallbackPending(local(flashAttempt));
 assert.equal(pending?.recovery,true);
 assert.equal(pending.sha,FP.stage.sha);
 assert.equal(fallbackMode('stage',id('SMK-37 Pro_000'),pending),'install');
 assert.throws(()=>fallbackMode('stock',id('SMK-37 Pro_000'),pending));
 for(const altered of [{...flashAttempt,sha:'bad'},{...flashAttempt,source:'SMK-37 Elite_19703'},{...flashAttempt,model:'SMK-37 Elite'}, {...flashAttempt,kind:'stock'}])
  assert.equal(fallbackPending(local(altered)),null);
 assert.equal(FP.stock.sha,'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0');
});
test('diag web page keeps explicit consent and immutable package selection',()=>{
 const h=fs.readFileSync('web/smkstage/install-d1/index.html','utf8');
 const c=fs.readFileSync('web/smkstage/install-d1/controller.js','utf8');
 assert.match(h,/0\.1\.1-d1/);
 assert.match(h,/CRASH DIAGNOSTIC/);
 for(const id of ['stageFile','stockFile','connect','consent','phrase','install','restore'])assert.match(h,new RegExp('id="'+id+'"'));
 assert.match(c,/file\.arrayBuffer\(\)/);
 assert.match(c,/service\.execute\(/);
});
