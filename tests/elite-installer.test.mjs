import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync,existsSync} from 'node:fs';
import {resolve} from 'node:path';
import {webcrypto,createHash} from 'node:crypto';
import {PINS,PENDING,validate,modeFor,readPending,InstallService} from '../web/install-elite/service.js';
globalThis.crypto??=webcrypto;
const raw=readFileSync('web/install-elite/'+PINS.rc.url), sha=b=>createHash('sha256').update(b).digest('hex');
const id=text=>({text,model:text.split('_')[0],version:Number(text.split('_')[1])});
const pending=(kind,source)=>({kind,source,sha:PINS[kind].sha});
const storage=initial=>{const data=new Map(initial?[[PENDING,JSON.stringify(initial)]]:[]);return {getItem:k=>data.get(k)||null,setItem:(k,v)=>data.set(k,v),removeItem:k=>data.delete(k)}};
test('Elite package pins, source model and manifest agree; official Pro pin stays unchanged',async()=>{
 const m=JSON.parse(readFileSync('web/install-elite/manifest.json'));
 assert.equal((await validate(raw,'rc')).product,'SMK-37 Elite_463');assert.equal(m.firmware_sha256,sha(raw));assert.equal(m.firmware_bytes,raw.length);assert.equal(m.hardware_tested,false);
 const {PINS:pro}=await import('../web/install/service.js');assert.equal(pro.rc.sha,'8b975a1f38fb09727e697107603680325cb5bcca2a90807ef46d62e683a1b16f');
 await assert.rejects(validate(readFileSync('web/install/'+pro.rc.url),'rc'));const bad=Buffer.from(raw);bad[22000]^=1;await assert.rejects(validate(bad,'rc'));
});
test('only known Elite sources and correct recovery operation are enabled',()=>{
 for(const source of ['SMK-37 Elite_016','SMK-37 Elite_463'])assert.equal(modeFor('rc',id(source),null),'install');
 for(const source of ['SMK-37 Elite_463','SMK-37 Elite_000'])assert.equal(modeFor('stock',id(source),null),'install');
 assert.throws(()=>modeFor('rc',id('SMK-37 Elite_000'),null));assert.throws(()=>modeFor('stock',id('SMK-37 Elite_016'),null));
 for(const model of ['SMK-37 Pro','FM-1','MKE-P37','ota-SMK-37 Pro'])for(const suffix of ['016','463','900'])assert.throws(()=>modeFor('rc',id(model+'_'+suffix),null));
 for(const kind of ['rc','stock']){
  const p=pending(kind,'SMK-37 Elite_463');assert.equal(modeFor(kind,id('ota-SMK-37 Elite_900'),p),'resume');
  for(const broken of [null,{...p,sha:'bad'},{...p,source:'SMK-37 Pro_463'},{...p,source:'ota-SMK-37 Elite_900'},{...p,kind:'other'}])assert.throws(()=>modeFor(kind,id('ota-SMK-37 Elite_900'),broken));
 }
 assert.equal(modeFor('stock',id('ota-SMK-37 Elite_016'),pending('stock','SMK-37 Elite_463')),'resume');
 assert.throws(()=>modeFor('rc',id('ota-SMK-37 Elite_016'),pending('rc','SMK-37 Elite_016')));
 assert.equal(readPending(storage(pending('rc','SMK-37 Pro_016'))),null);
});
const stockPath=process.env.ELITE_STOCK;
test('real Elite V016 validated; install guard and repeated resume preserve original source',{skip:!stockPath},async()=>{
 const stock=readFileSync(stockPath);assert.equal((await validate(stock,'stock')).product,'SMK-37 Elite_016');
 let device='SMK-37 Elite_016',loader='ota-SMK-37 Elite_900',interrupt=false,finds=0,writes=0;
 class Mock {
  constructor(a,l,{guard}={}){this.guard=guard}
  async find(){finds++;return {id:id(device),link:{close(){}}}}
  async install(image,product,step,anyModel){assert.equal(anyModel,undefined);this.guard('start',id(device));this.guard('write',id(loader));writes++;if(interrupt)throw Error('disconnected')}
  async resume(image,step,opts){this.guard('write',id(loader));assert.equal(opts.product,PINS.rc.product);writes++;if(interrupt)throw Error('disconnected');return true}
 }
 const s=storage(),service=new InstallService({},s,()=>{},Mock);
 interrupt=true;await assert.rejects(service.execute('rc',raw,stock));assert.equal(readPending(s).source,'SMK-37 Elite_016');
 device='ota-SMK-37 Elite_900';await assert.rejects(service.execute('rc',raw,stock));assert.equal(readPending(s).source,'SMK-37 Elite_016');
 interrupt=false;assert.equal(await service.execute('rc',raw,stock),PINS.rc.product);assert.equal(readPending(s),null);
 device='SMK-37 Pro_016';const before=writes;await assert.rejects(service.execute('rc',raw,stock));assert.equal(writes,before);
 device='SMK-37 Elite_016';loader='ota-SMK-37 Pro_900';await assert.rejects(service.execute('rc',raw,stock));assert.equal(writes,before);
 const q=finds;await assert.rejects(service.execute('rc',raw,new Uint8Array(10)));assert.equal(finds,q);
 loader='ota-SMK-37 Elite_016';device='SMK-37 Elite_463';assert.equal(await service.execute('stock',raw,stock),PINS.stock.product);
});
test('standalone test page links resolve and no stock firmware is distributed',()=>{
 const h=readFileSync('web/install-elite/index.html','utf8');assert(h.includes('noindex,nofollow,noarchive'));assert(h.includes('ainda não validados'));assert(!h.includes('SMK-37 Pro_016'));
 for(const m of h.matchAll(/(?:href|src)="([^"]+)"/g)){const u=m[1].split('#')[0];if(!u||/^https?:/.test(u))continue;assert(existsSync(resolve('web/install-elite',u)),u)}
 const c=readFileSync('web/install-elite/controller.js','utf8');assert(c.includes("model:'SMK-37 Elite'"));assert(c.includes("navigator.locks.request('smktron-firmware-update'"));
});
