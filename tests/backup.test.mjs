// SPDX-License-Identifier: GPL-3.0-only
import assert from 'node:assert/strict';
import {spawn, execFileSync} from 'node:child_process';
import {createHash} from 'node:crypto';
import {mkdtemp, readFile, rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {createInterface} from 'node:readline';
import test from 'node:test';
import {BackupClient, crc32, makeFile, parseFile} from '../web/backup/client.js';
import {preview} from '../web/backup/firmware.js';
import {filesOk, logicalImage, productOf} from '../web/install/package.js';

test('preview package matches its pinned digest and Pro identity', async () => {
    const pkg = await readFile(new URL(preview.url, new URL('../web/backup/', import.meta.url)));
    assert.equal(pkg.length, preview.size);
    assert.equal(createHash('sha256').update(pkg).digest('hex'), preview.sha);
    assert.equal(productOf(pkg), 'SMK-37 Pro_464');
    assert.ok(filesOk(logicalImage(pkg)));
});

test('browser and firmware exchange a saved bank and retry a partial restore', async () => {
    const dir = await mkdtemp(join(tmpdir(), 'tron-backup-'));
    let process;
    try {
        const executable = join(dir, 'device');
        execFileSync('cc', ['-O2', '-fno-builtin', '-I', 'firmware/firmware/gen',
            '-I', 'firmware/firmware/src', 'tests/backup-device.c', '-o', executable]);
        process = spawn(executable);
        const lines = createInterface({input: process.stdout})[Symbol.asyncIterator]();
        const next = async () => {
            const {value, done} = await lines.next();
            assert.equal(done, false, 'simulated device exited');
            return value;
        };
        const client = new BackupClient({
            drain() {},
            send(message) { return process.stdin.write(Buffer.from(message).toString('hex') + '\n'); },
            async read() { return Buffer.from(await next(), 'hex'); },
        });
        const device = async command => {
            process.stdin.write(command + '\n');
            assert.equal(await next(), 'ok');
        };
        assert.equal((await client.check()).state, 0);
        const original = await client.exportBank();
        const file = parseFile(makeFile(original));
        assert.deepEqual(file.scenes, [4]);
        assert.deepEqual(file.chords, [3]);
        assert.throws(() => parseFile(makeFile(original).replace(/"crc32": \d+/, '"crc32": 0')), {code: 'file'});
        const invalid = original.slice();
        invalid[1536] = 7;
        await assert.rejects(client.restoreBank(invalid), {code: 4});
        assert.deepEqual(await client.exportBank(), original);

        const changed = original.slice();
        changed[0] = 0;
        changed[1538] = 0;
        changed[1366] = 40;
        changed[1604] = 2; // Selected MIDI profile.
        await device('fail');
        await assert.rejects(client.restoreBank(changed), {code: 5});
        assert.deepEqual(await client.check(), {state: 2, checksum: crc32(changed)});
        await assert.rejects(client.restoreBank(original), {code: 4});
        await device('recover');
        await client.restoreBank(changed);
        assert.equal((await client.check()).state, 1);
        await assert.rejects(client.exportBank(), {code: 6});
        await device('reboot');
        assert.deepEqual(await client.exportBank(), changed);
    } finally {
        process?.kill();
        await rm(dir, {recursive: true, force: true});
    }
});
