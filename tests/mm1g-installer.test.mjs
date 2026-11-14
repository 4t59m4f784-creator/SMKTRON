import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {validate,modeFor} from '../web/install-mm1g/service.js';
import {validate as rollbackValidate,modeFor as rollbackMode} from '../web/rollback-mm1f/service.js';
test('MM1g is pinned; direct update and official rollback accept 460/461 only on Pro',async()=>{
 const fw=new Uint8Array(readFileSync('web/install-mm1g/firmware/smktron-0.3.0-mm1g-pro.fwsc'));
 assert.equal((await validate(fw,'rc')).product,'SMK-37 Pro_461');
 const id=v=>({text:'SMK-37 Pro_'+v,model:'SMK-37 Pro'});
 for(const v of ['016','453','460','461'])assert.equal(modeFor('rc',id(v)),'install');
 assert.equal(rollbackMode('rc',id('461')),'install');
 const official=new Uint8Array(readFileSync('web/install/firmware/smktron-0.3.0-mm1f-pro.fwsc'));
 assert.equal((await rollbackValidate(official,'rc')).product,'SMK-37 Pro_460');
 assert.throws(()=>modeFor('rc',{text:'SMK-37 Elite_461',model:'SMK-37 Elite'}));
 assert.throws(()=>modeFor('rc',id('999')));
 assert.throws(()=>modeFor('rc',{text:'ota-SMK-37 Pro_900',model:'ota-SMK-37 Pro'}));
 assert.equal(modeFor('rc',{text:'ota-SMK-37 Pro_900',model:'ota-SMK-37 Pro'},{kind:'rc',source:'SMK-37 Pro_460'}),'resume');
 fw[2000]^=1;await assert.rejects(validate(fw,'rc'));
});
