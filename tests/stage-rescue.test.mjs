import test from 'node:test';
import assert from 'node:assert/strict';
import {PINS,readPending,modeFor,PENDING} from '../web/smkstage/install-011/service.js';
const device={text:'SMK-37 Pro_000',model:'SMK-37 Pro'};
const failed={model:'SMK-37 Pro',kind:'stage',sha:'2d992f98864558b24d61a007e20c7c6c192a441489c5be4c90c2ca20881cd97b',source:'SMK-37 Pro_19703'};
const store=p=>({getItem:k=>k===PENDING?JSON.stringify(p):null});
test('rescue: exact failed 0.1.2 source authorizes only pinned playable 0.1.1',()=>{
 const pending=readPending(store(failed));
 assert.equal(pending.recovery,true);
 assert.equal(pending.sha,PINS.stage.sha);
 assert.equal(pending.source,device.text);
 assert.equal(modeFor('stage',device,pending),'install');
 assert.throws(()=>modeFor('stock',device,pending));
});
test('rescue blocked without evidence and for unrelated devices',()=>{
 assert.throws(()=>modeFor('stage',device,null));
 for(const v of [
  {...failed,sha:'0000'}, {...failed,kind:'stock'}, {...failed,source:'SMK-37 Pro_19668'},
  {...failed,model:'SMK-37 Elite'}
 ])assert.throws(()=>modeFor('stage',device,readPending(store(v))));
 assert.throws(()=>modeFor('stage',{text:'SMK-37 Elite_000',model:'SMK-37 Elite'},readPending(store(failed))));
 assert.throws(()=>modeFor('stage',{text:'FM-1_000',model:'FM-1'},readPending(store(failed))));
});
test('previous 0.1.1 package stays unchanged and pinned',()=>{
 assert.equal(PINS.stage.sha,'3b735c88ab65482153a903bfab9988b78b28c4f19ee064cfdb189dc57ca02c0c');
 assert.equal(PINS.stage.product,'SMK-37 Pro_19703');
 assert.equal(PINS.stage.size,651339);
});
