// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
// SMK-37 adaptations: genkigenki. Extracted from genkigenki/smk37, bf94853f7f28cdecfae044057962fa9a2821b773.
const BLOCKS = 20, BLK = 0x30, KEEP = 0x2F, MAX_BLOCKS = 64;

const printable = (m, i) => { const c = (m - i - 1) & 0xFF; return c >= 0x20 && c < 0x7F; };

// the identity blocks: 20, or more when the identity is longer (the Starrykey 37 Play V016's "STARRYKEY-37 PLAY_016"
// takes 21): further identity markers count only when unused 0x7D markers follow them (the 36-block layout)
function identityBlocks(fwsc) {
  let n = BLOCKS;
  while (n < MAX_BLOCKS && (n + 1) * BLK <= fwsc.length && fwsc[n * BLK + KEEP] !== 0x7D && printable(fwsc[n * BLK + KEEP], n)) n++;
  return n > BLOCKS && (n + 1) * BLK <= fwsc.length && fwsc[n * BLK + KEEP] === 0x7D ? n : BLOCKS;
}

// interleave blocks: the identity blocks, then more while the marker is the unused 0x7D (the stock SMK-37 Pro,
// MKE-P37 and Starrykey 37 Play V016 have 36)
function blockCount(fwsc) {
  let n = identityBlocks(fwsc);
  while (n < MAX_BLOCKS && (n + 1) * BLK <= fwsc.length && fwsc[n * BLK + KEEP] === 0x7D) n++;
  return n;
}

// the package identity ("FM-1_904", "SMK-37 Pro_016"): one marker byte after each identity block (0x7D: none);
// past the first 20 only when the package really has those blocks (logicalImage's count)
function productOf(fwsc) {
  if (fwsc.length < BLOCKS * BLK) throw new Error("not an FM-1 package (too short)");
  const ib = identityBlocks(fwsc), n = ib === BLOCKS ? BLOCKS : Math.min(ib, layout(fwsc).blocks);
  let s = "";
  for (let i = 0; i < n; i++) {
    const m = fwsc[i * BLK + KEEP];
    if (m !== 0x7D) s += String.fromCharCode((m - i - 1) & 0xFF);
  }
  return s;
}

function deinterleave(fwsc, blocks) {
  const out = new Uint8Array(fwsc.length - blocks);
  for (let i = 0; i < blocks; i++) out.set(fwsc.subarray(i * BLK, i * BLK + KEEP), i * KEEP);
  out.set(fwsc.subarray(blocks * BLK), blocks * KEEP);
  return out;
}

function crc16(d, c = 0) {                 // CRC-16/XMODEM, as the package and the device use
  for (const b of d) {
    c ^= b << 8;
    for (let k = 0; k < 8; k++) c = c & 0x8000 ? ((c << 1) ^ 0x1021) & 0xFFFF : (c << 1) & 0xFFFF;
  }
  return c;
}

function jlDec(b) {                        // JieLi ENC stream cipher (its own inverse)
  const out = Uint8Array.from(b);
  let key = 0xFFFF;
  for (let i = 0; i < out.length; i++) {
    out[i] ^= key & 0xFF;
    key = ((key << 1) ^ (key & 0x8000 ? 0x1021 : 0)) & 0xFFFF;
  }
  return out;
}

// the logical image's flash.bin (type 0) and ota.bin (type 100) match their CRCs (as fm1_install.py)
function filesOk(img) {
  if (img.length < 0x40) return false;
  const h = jlDec(img.subarray(0, 0x40)), rd = (a, o, n) => a.slice(o, o + n).reduce((v, x, i) => v + x * 2 ** (8 * i), 0);
  if (crc16(h.subarray(2, 0x40)) !== rd(h, 0, 2)) return false;
  const count = rd(h, 8, 2);
  if (count < 1 || count > 8 || img.length < 0x40 + 0x50 * count) return false;
  for (let i = 0; i < count; i++) {
    const e = jlDec(img.subarray(0x40 + i * 0x50, 0x90 + i * 0x50));
    const typ = rd(e, 0, 2), off = rd(e, 8, 4), size = rd(e, 12, 4);
    if (off + size > img.length) return false;
    if ((typ === 0 || typ === 100) && crc16(img.subarray(off, off + size)) !== rd(e, 4, 2)) return false;
  }
  return true;
}

// the image the device addresses during an update: the .fwsc without the marker bytes (20 blocks, or the
// detected count when the files' CRCs agree: a 20-block package can have 0x7D after block 20 by chance)
function layout(fwsc) {
  const n = blockCount(fwsc);
  if (n !== BLOCKS) {
    const img = deinterleave(fwsc, n);
    if (filesOk(img) || !filesOk(deinterleave(fwsc, BLOCKS))) return { blocks: n, image: img };
  }
  return { blocks: BLOCKS, image: deinterleave(fwsc, BLOCKS) };
}
function logicalImage(fwsc) { return layout(fwsc).image; }


export { productOf, logicalImage, filesOk, layout, crc16 };
