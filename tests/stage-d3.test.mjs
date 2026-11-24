import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {PINS as DP,modeFor as dm,PENDING as KEY,readPending as rd} from '../web/smkstage/install-d3/service.js';
import {PINS as BP,modeFor as bm,readPending as rb} from '../web/smkstage/install-011/service.js';
const hash='02504176970bc7d34b04cb0af2a23c1a56accfa4f8af67a442102e7557157405';
const id=t=>({text:t,model:t.startsWith('ota-')?'ota-SMK-37 Pro':'SMK-37 Pro'});
const pending={model:'SMK-37 Pro',kind:'stage',sha:hash,source:'SMK-37 Pro_19703'};
const local=t=>({getItem:k=>k===KEY?JSON.stringify(t):null});
test('D3 pinned to independently compiled 651339-byte SMK-37 Pro image',()=>{
 assert.equal(DP.stage.sha,hash);
 assert.equal(DP.stage.size,651339);
 assert.equal(DP.stage.product,'SMK-37 Pro_19703');
 assert.equal(DP.stock.sha,'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0');
 assert.equal(DP.stage.name,'SMKSTAGE B3 Alpha 0.1.3-d3');
});
test('D3 refuses USB Rescue and wrong hardware; never silently resumes failed firmware',()=>{
 assert.equal(dm('stage',id('SMK-37 Pro_19703'),null),'install');
 assert.throws(()=>dm('stage',id('SMK-37 Pro_000'),null));
 assert.throws(()=>dm('stage',id('SMK-37 Pro_000'),{...pending,recovery:true}));
 assert.throws(()=>dm('stage',{text:'SMK-37 Elite_19703',model:'SMK-37 Elite'},null));
 assert.equal(rd(local(pending))?.sha,hash);
});
test('0.1.1 rescue permitted only for exact same-browser D3 transaction',()=>{
 const r=rb(local(pending));
 assert.equal(r?.recovery,true);
 assert.equal(r.sha,BP.stage.sha);
 assert.equal(bm('stage',id('SMK-37 Pro_000'),r),'install');
 assert.throws(()=>bm('stock',id('SMK-37 Pro_000'),r));
 for(const t of [{...pending,sha:'bad'},{...pending,source:'SMK-37 Elite_19703'},{...pending,model:'FM-1'},{...pending,kind:'stock'}])assert.equal(rb(local(t)),null);
});
test('D3 requires user-selected local fwsc, confirmation, and D3 code in UI',()=>{
 const h=fs.readFileSync('web/smkstage/install-d3/index.html','utf8');
 const c=fs.readFileSync('web/smkstage/install-d3/controller.js','utf8');
 assert.match(h,/0\.1\.3-d3/);
 for(const x of ['stageFile','stockFile','connect','consent','phrase','install','restore'])assert.match(h,new RegExp('id="'+x+'"'));
 assert.match(c,/file\.arrayBuffer\(\)/);
 assert.match(c,/service\.execute\(/);
});
