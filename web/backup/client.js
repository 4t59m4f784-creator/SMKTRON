// SPDX-License-Identifier: GPL-3.0-only
import {pack7, unpack7} from '../install/protocol.js';
import {crc16} from '../install/package.js';

export const SIZE = 2596;
const BLOCK = 256;
const MAGIC = 'TRONBK1';
const FILE_TYPE = 'smktron-backup';
const encoder = new TextEncoder();
const word = value => [value & 255, value >>> 8 & 255];
const dword = value => [...word(value), ...word(value >>> 16)];
const readWord = (data, offset) => data[offset] | data[offset + 1] << 8;
const readDword = data => new DataView(data.buffer, data.byteOffset, data.byteLength).getUint32(0, true);
const sum = bytes => bytes.reduce((total, byte) => total + byte, 0);

export class BackupError extends Error {
    constructor(code) {
        super(code);
        this.code = code;
    }
}

function packet(command, payload) {
    return [0xf0, ...pack7([0, 0x59, command, ...word(payload.length), 0, ...payload, ~sum(payload) & 255]), 0xf7];
}

function unpackReply(wire, command, seq) {
    if (wire[0] !== 0xf0 || wire.at(-1) !== 0xf7) return null;
    const decoded = unpack7(wire.slice(1, -1));
    const body = decoded.slice(6, -1);
    if (decoded[0] !== 0 || decoded[1] !== 0x59 || decoded[2] !== 0x63 ||
        decoded[5] !== 0 || readWord(decoded, 3) !== body.length || body.length < 6 ||
        decoded.at(-1) !== (~sum(body) & 255) || body[0] !== command || readWord(body, 1) !== seq ||
        crc16(body.slice(0, -2)) !== readWord(body, body.length - 2)) return null;
    return {status: body[3], data: Uint8Array.from(body.slice(4, -2))};
}

export function crc32(bytes) {
    let crc = 0xffffffff;
    for (const byte of bytes) {
        crc ^= byte;
        for (let bit = 0; bit < 8; bit++) crc = (crc >>> 1) ^ (crc & 1 ? 0xedb88320 : 0);
    }
    return (crc ^ 0xffffffff) >>> 0;
}

export function makeFile(bank) {
    return JSON.stringify({
        type: FILE_TYPE,
        version: 1,
        createdAt: new Date().toISOString(),
        crc32: crc32(bank),
        data: btoa(String.fromCharCode(...bank)),
    }, null, 2);
}

export function parseFile(text) {
    try {
        const file = JSON.parse(text);
        if (file.type !== FILE_TYPE || file.version !== 1 || typeof file.data !== 'string') throw Error();
        const bank = Uint8Array.from(atob(file.data), c => c.charCodeAt(0));
        if (bank.length !== SIZE || crc32(bank) !== file.crc32 || bank[1] || bank[2] || bank[3]) throw Error();
        return {
            bank,
            scenes: [...Array(8).keys()].filter(i => bank[0] & (1 << i)).map(i => i + 1),
            chords: [...Array(8).keys()].filter(i => bank[1536 + i]).map(i => i + 1),
        };
    } catch {
        throw new BackupError('file');
    }
}

export class BackupClient {
    constructor(link) {
        this.link = link;
        this.seq = new Uint16Array(1);
        crypto.getRandomValues(this.seq);
    }

    async command(op, args = []) {
        const seq = ++this.seq[0] & 0xffff;
        const payload = [op, ...word(seq), ...args];
        const request = packet(0x62, [...payload, ...word(crc16(payload))]);
        this.link.drain();
        for (let retry = 0; retry < 3; retry++) {
            if (!this.link.send(request)) throw new BackupError('usb');
            const end = Date.now() + 1800;
            while (Date.now() < end) {
                const wire = await this.link.read(end - Date.now());
                if (!wire) break;
                const reply = unpackReply(wire, op, seq);
                if (!reply) continue;
                if (reply.status) throw new BackupError(reply.status);
                return reply.data;
            }
        }
        throw new BackupError('timeout');
    }

    async check() {
        const info = await this.command(0);
        if (info.length !== 14 || !encoder.encode(MAGIC).every((v, i) => info[i] === v) || readWord(info, 7) !== SIZE)
            throw new BackupError('model');
        return {state: info[9], checksum: readDword(info.subarray(10))};
    }

    async exportBank(progress = () => {}) {
        const checksum = await this.command(1);
        if (checksum.length !== 4) throw new BackupError('data');
        const bank = new Uint8Array(SIZE);
        for (let offset = 0; offset < SIZE; offset += BLOCK) {
            const block = await this.command(2, word(offset));
            if (block.length !== Math.min(BLOCK, SIZE - offset)) throw new BackupError('data');
            bank.set(block, offset);
            progress((offset + block.length) / SIZE);
        }
        if (crc32(bank) !== readDword(checksum)) throw new BackupError('data');
        return bank;
    }

    async restoreBank(bank, progress = () => {}) {
        if (bank.length !== SIZE) throw new BackupError('file');
        const checksum = dword(crc32(bank));
        await this.command(3, checksum);
        for (let offset = 0; offset < bank.length; offset += BLOCK) {
            const block = bank.subarray(offset, offset + BLOCK);
            await this.command(4, [...word(offset), ...block]);
            progress((offset + block.length) / bank.length);
        }
        await this.command(5, checksum);
    }
}
