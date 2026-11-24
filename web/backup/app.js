// SPDX-License-Identifier: GPL-3.0-only
import {Updater} from '../install/protocol.js';
import {filesOk, logicalImage, productOf} from '../install/package.js';
import {PINS} from '../install/service.js';
import {BackupClient, BackupError, crc32, makeFile, parseFile} from './client.js';
import {preview} from './firmware.js';

const $ = id => document.getElementById(id);
const installed = preview.product;
const official = PINS.rc;
const versions = {install: preview, rollback: {...official, url: '../install/firmware/smktron-0.3.0-ml1-pro.fwsc'}};
const resumeKey = 'smktron.backup.preview.pending';
const messages = {
    1: 'O teclado recusou o comando.',
    2: 'Pare PLAY e REC, saia do MODO MIDI, solte as teclas, os pads e o pedal e aguarde o áudio terminar.',
    3: 'A sessão expirou. Repita a operação.',
    4: 'O banco contém dados incompatíveis ou corrompidos.',
    5: 'O teclado informou uma falha de memória. Se aconteceu na restauração, reconecte e tente novamente com o mesmo arquivo.',
    6: 'Desligue e ligue o teclado para carregar o banco gravado.',
    usb: 'USB desconectado. Reconecte o teclado.',
    timeout: 'O teclado não respondeu. Se ocorreu ao gravar, reconecte e repita com o mesmo arquivo.',
    model: 'Conecte um SMK-37 Pro com o firmware de teste 0.3.0-bk1.',
    file: 'O arquivo está corrompido ou pertence a outro formato.',
    data: 'A verificação da leitura falhou. Repita a exportação.',
};

let access, device, selectedFile, exported, busy, restartRequired;
const tell = (id, value) => { $(id).textContent = value; };
const errorMessage = error => messages[error.code] ?? error.message;
const connected = () => device && !device.link.lost;
function controls() {
    $('connect').disabled = busy;
    $('export').disabled = busy || !connected() || restartRequired;
    $('file').disabled = busy;
    $('restore').disabled = busy || !connected() || !selectedFile || restartRequired;
    $('install').disabled = busy;
    $('rollback').disabled = busy;
}

async function task(status, action) {
    if (busy) return;
    busy = true;
    controls();
    try {
        await action();
    } catch (error) {
        tell(status, errorMessage(error));
    } finally {
        busy = false;
        $('progress').hidden = true;
        controls();
    }
}

async function midi() {
    if (!navigator.requestMIDIAccess) throw Error('Abra esta página no Chrome ou Edge, via HTTPS ou localhost.');
    if (!access) {
        access = await navigator.requestMIDIAccess({sysex: true});
        access.addEventListener('statechange', () => {
            if (device?.link.lost) {
                exported = undefined;
                tell('device', 'Teclado desconectado');
            }
            controls();
        });
    }
    return access;
}

function progress(value) {
    $('progress').hidden = false;
    $('progress').value = value;
}

$('connect').onclick = () => task('status', async () => {
    device?.link.close();
    device = undefined;
    exported = undefined;
    restartRequired = false;
    tell('device', 'Buscando teclado…');
    try {
        const found = await new Updater(await midi()).find(id => id.text === installed);
        if (!found) throw new BackupError('model');
        const client = new BackupClient(found.link);
        let info;
        try {
            info = await client.check();
        } catch (error) {
            found.link.close();
            throw error;
        }
        device = {...found, client};
        tell('device', 'SMK-37 Pro conectado · 0.3.0-bk1');
        restartRequired = info.state === 1;
        tell('status', restartRequired ? messages[6] : info.state === 2 ? messages[5] : '');
    } catch (error) {
        tell('device', 'Aguardando conexão');
        throw error;
    }
});

function download(bank) {
    const url = URL.createObjectURL(new Blob([makeFile(bank)], {type: 'application/json'}));
    const link = document.createElement('a');
    link.href = url;
    link.download = 'SMKTRON-' + new Date().toISOString().slice(0, 19).replaceAll(':', '-') + '.smktron';
    link.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
}

$('export').onclick = () => task('status', async () => {
    tell('status', 'Lendo o banco salvo…');
    const bank = await device.client.exportBank(progress);
    download(bank);
    exported = bank;
    tell('status', 'Backup verificado. Confira o arquivo na pasta de downloads.');
});

$('file').onchange = () => task('status', async () => {
    selectedFile = undefined;
    tell('summary', '');
    const file = $('file').files[0];
    if (!file) return;
    if (file.size > 8192) throw new BackupError('file');
    const parsed = parseFile(await file.text());
    selectedFile = parsed;
    tell('summary', 'Cenas: ' + (parsed.scenes.join(', ') || 'nenhuma') +
        ' · Acordes: ' + (parsed.chords.join(', ') || 'nenhum') + ' · Inclui Boot Save e mapas MIDI');
    tell('status', '');
});

$('restore').onclick = () => task('status', async () => {
    const info = await device.client.check();
    if (info.state === 1) throw new BackupError(6);
    if (info.state === 2) {
        if (info.checksum !== crc32(selectedFile.bank)) throw Error('Selecione o mesmo arquivo usado na restauração interrompida.');
    } else {
        if (!exported) throw Error('Clique em Baixar backup e guarde a cópia atual antes de restaurar.');
        tell('status', 'Verificando a cópia atual…');
        const latest = await device.client.exportBank(progress);
        if (!latest.every((value, offset) => value === exported[offset])) {
            exported = undefined;
            throw Error('O banco mudou desde a exportação. Baixe um novo backup antes de restaurar.');
        }
    }
    if (!confirm('Substituir todas as cenas, acordes e ajustes salvos pelo arquivo selecionado?')) {
        tell('status', 'Restauração cancelada.');
        return;
    }
    tell('status', 'Transferindo arquivo e gravando o banco…');
    await device.client.restoreBank(selectedFile.bank, progress);
    restartRequired = true;
    exported = undefined;
    tell('status', 'Restauração verificada. Desligue e ligue o teclado para carregar o banco.');
});

async function install(kind) {
    const target = versions[kind];
    const source = kind === 'install' ? official.product : installed;
    device?.link.close();
    device = undefined;
    exported = undefined;
    tell('device', 'Aguardando conexão');
    tell('installation', 'Verificando o pacote…');
    const response = await fetch(target.url);
    if (!response.ok) throw Error('Pacote indisponível.');
    const raw = new Uint8Array(await response.arrayBuffer());
    const sha = Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256', raw)),
        byte => byte.toString(16).padStart(2, '0')).join('');
    const image = logicalImage(raw);
    if (raw.length !== target.size || sha !== target.sha || productOf(raw) !== target.product || !filesOk(image))
        throw Error('Falha na verificação do pacote.');
    const loader = 'ota-SMK-37 Pro_900';
    const updater = new Updater(await midi(), undefined, {guard(phase, id) {
        if (id.text !== (phase === 'start' ? source : loader)) throw Error('A identidade do teclado mudou.');
    }});
    const found = await updater.find();
    if (!found) throw Error('Teclado não encontrado.');
    found.link.close();
    const resuming = found.id.text === loader && localStorage.getItem(resumeKey) === target.sha;
    if (!resuming && found.id.text !== source) throw Error('A operação exige ' + source + '.');
    if (!confirm((resuming ? 'Retomar instalação de ' : 'Instalar ') + target.name + '?')) {
        tell('installation', 'Operação cancelada.');
        return;
    }
    localStorage.setItem(resumeKey, target.sha);
    const step = (name, detail) => tell('installation', ({
        start: 'Preparando dispositivo…', verify: 'Verificando pacote…', loader: 'Aguardando o carregador…',
        write: 'Gravando firmware. Mantenha o USB conectado…', reboot: 'Aguardando o reinício…', done: 'Firmware conferido.',
    })[name] ?? detail);
    if (resuming) {
        if (!await updater.resume(image, step, {product: target.product})) throw Error('Não foi possível retomar a instalação.');
    } else {
        await updater.install(image, target.product, step);
    }
    localStorage.removeItem(resumeKey);
    tell('installation', target.name + ' instalado.');
}

$('install').onclick = () => task('installation', () => install('install'));
$('rollback').onclick = () => task('installation', () => install('rollback'));
controls();
