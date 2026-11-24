import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {PINS as D4,modeFor as d4Mode,readPending as d4Read,PENDING as D4_KEY} from '../web/smkstage/install-d4/service.js';
import {PINS as BASE,modeFor as baseMode,readPending as baseRead} from '../web/smkstage/install-011/service.js';
const HASH='e0ef655f4f02a9c1d0d966b280e995e213a99c96afc902e9facaf43f8b10b26a';
const id=text=>({text,model:text.startsWith('ota-')?'ota-SMK-37 Pro':'SMK-37 Pro'});
const failed={model:'SMK-37 Pro',kind:'stage',sha:HASH,source:'SMK-37 Pro_19703'};
const storage=value=>({getItem:key=>key===D4_KEY?JSON.stringify(value):null});
test('D4 binary is pinned, not D3 or stable 0.1.1',()=>{
 assert.equal(D4.stage.name,'SMKSTAGE B3 Alpha 0.1.3-d4');
 assert.equal(D4.stage.product,'SMK-37 Pro_19703');
 assert.equal(D4.stage.size,651339);
 assert.equal(D4.stage.sha,HASH);
 assert.equal(D4.stock.sha,'2d98ce72530e71d617384963423820536f094501299e2cfec0a8d3c3fb6bcfb0');
 assert.notEqual(D4.stage.sha,BASE.stage.sha);
});
test('D4 refuses USB Rescue and foreign hardware',()=>{
 assert.equal(d4Mode('stage',id('SMK-37 Pro_19703'),null),'install');
 assert.throws(()=>d4Mode('stage',id('SMK-37 Pro_000'),null));
 assert.throws(()=>d4Mode('stage',id('SMK-37 Pro_000'),{...failed,recovery:true}));
 assert.throws(()=>d4Mode('stage',{text:'SMK-37 Elite_19703',model:'SMK-37 Elite'},null));
 assert.equal(d4Read(storage(failed))?.sha,HASH);
});
test('fallback from same-browser D4 is the known-good 0.1.1, and nothing else',()=>{
 const p=baseRead(storage(failed));
 assert.equal(p?.recovery,true);
 assert.equal(p?.sha,BASE.stage.sha);
 assert.equal(baseMode('stage',id('SMK-37 Pro_000'),p),'install');
 assert.throws(()=>baseMode('stock',id('SMK-37 Pro_000'),p));
 for(const v of [{...failed,sha:'0000'},{...failed,model:'FM-1'},{...failed,kind:'stock'},{...failed,source:'SMK-37 Elite_19703'}])
  assert.equal(baseRead(storage(v)),null);
});
test('diagnostic UI requires local files and explicit consent',()=>{
 const h=fs.readFileSync('web/smkstage/install-d4/index.html','utf8');
 const c=fs.readFileSync('web/smkstage/install-d4/controller.js','utf8');
 assert.match(h,/0\.1\.3-d4/);
 assert.match(h,/CPU0/);
 for(const x of ['stageFile','stockFile','connect','consent','phrase','install','restore'])assert.match(h,new RegExp('id="'+x+'"'));
 assert.match(c,/file\.arrayBuffer\(\)/);
 assert.match(c,/service\.execute\(/);
});
