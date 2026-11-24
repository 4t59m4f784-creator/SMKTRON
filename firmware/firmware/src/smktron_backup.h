/* SPDX-License-Identifier: GPL-3.0-only */
/* USB SysEx 0x62/0x63. Only logical SMKTRON records may be read or written. */
typedef struct {
    sc_bank_t scenes;
    mel_set_t boot;
    ch_bank_t chords;
    mm_store_t midi;
} backup_bank_t;

_Static_assert(sizeof(backup_bank_t) == 2596u, "BK1 layout");
_Static_assert(__builtin_offsetof(backup_bank_t, boot) == 1364u, "BK1 boot");
_Static_assert(__builtin_offsetof(backup_bank_t, chords) == 1534u, "BK1 chords");
_Static_assert(__builtin_offsetof(backup_bank_t, midi) == 1596u, "BK1 MIDI");

enum { BK_INFO, BK_EXPORT, BK_READ, BK_UPLOAD, BK_WRITE, BK_COMMIT };
enum { BK_OK, BK_INVALID, BK_BUSY, BK_EXPIRED, BK_DATA, BK_FLASH, BK_RESTART };
enum { BK_NONE, BK_READING, BK_WRITING };
#define BK_BLOCK 256u
#define BK_JOURNAL_MAGIC 0x324B4254u /* TBK2 */
_Static_assert(sizeof(backup_bank_t)+8u <= 4096u, "Backup journal fits one sector");

static backup_bank_t backup_bank __attribute__((section(".pool")));
static unsigned backup_mode, backup_received;
static uint32_t backup_deadline, backup_crc, backup_committed_crc;
static unsigned backup_commit_status;
static uint8_t backup_reply[8u + BK_BLOCK + 7u], backup_wire[312u];

static void backup_snapshot(void)
{
    memset(&backup_bank, 0, sizeof backup_bank);
    backup_bank.scenes = sc_bank;
    for (unsigned i = 0; i < 8u; i++) {
        if (backup_bank.scenes.mask & (1u << i))
            gr_migrate(&backup_bank.scenes.slot[i]);
    }
    if (boot_load(&backup_bank.boot)) {
        mel_set_t *boot = &backup_bank.boot;
        boot->ver = SET_VER;
        boot->np = NP;
        boot->half = 60u; /* Default master output is 70%. */
        boot->bypass = 12u;
        boot->master = 921u;
        for (unsigned i = 0; i < NP; i++)
            boot->p[i] = PINFO[i][2];
    }
    backup_bank.chords = ch_bank;
    mm_load_saved(&backup_bank.midi);
    backup_bank.midi.seq = 0; /* A restored map gets the destination's next generation. */
}

static int backup_valid(void)
{
    if (backup_bank.scenes.mask & ~255u)
        return 0;
    for (unsigned i = 0; i < 8u; i++) {
        if ((backup_bank.scenes.mask & (1u << i)) && !sc_valid(&backup_bank.scenes.slot[i]))
            return 0;
    }
    mel_set_t boot = backup_bank.boot;
    if (boot.half > 201u || boot.bypass > 127u)
        return 0;
    boot.half &= 1u;
    boot.bypass &= 7u;
    return sc_valid(&boot) && ch_valid(&backup_bank.chords) && mm_valid(&backup_bank.midi);
}

static void backup_screen(const char *message)
{
    lcd_fill(0, 0, 240, 240, C_BLACK);
    draw_text_box(0, 96, 240, &FONT_S, "SMKTRON BACKUP", C_WHITE, 1);
    draw_text_box(0, 128, 240, &FONT_S, message, C_WHITE, 1);
}

static int backup_restore(void)
{
    uint32_t took;
    /* Discover active sectors before a power-loss replay. Each twin bank then
     * writes to its other sector using the existing header-last writer. */
    if (sc_store_load(&sc_bank, sizeof sc_bank) ||
        memcmp(&sc_bank, &backup_bank.scenes, sizeof sc_bank)) {
        if (sc_store_save(&backup_bank.scenes, sizeof backup_bank.scenes)) return -1;
    }
    ch_init();
    if (memcmp(&ch_bank, &backup_bank.chords, sizeof ch_bank) &&
        ch_store_store_save(&backup_bank.chords, sizeof backup_bank.chords)) return -1;

    mm_store_t current, target = backup_bank.midi;
    unsigned slot = mm_load_saved(&current) ^ 1u;
    target.seq = current.seq ? current.seq : 1u;
    if (memcmp(&current, &target, sizeof current)) {
        target.seq = current.seq + 1u;
        if (!target.seq) target.seq = 1u;
        if (mm_store_save(slot, &target, sizeof target) ||
            mm_store_load(slot, &current, sizeof current) ||
            memcmp(&current, &target, sizeof current)) return -1;
    }

    mel_set_t boot;
    if (settings_load(&boot, sizeof boot) || memcmp(&boot, &backup_bank.boot, sizeof boot)) {
        if (settings_save(&backup_bank.boot, sizeof backup_bank.boot) ||
            settings_load(&boot, sizeof boot) || memcmp(&boot, &backup_bank.boot, sizeof boot)) return -1;
    }
    return fl_erase4k(BACKUP_JOURNAL_AT, &took);
}

static int backup_stage(void)
{
    uint32_t header[2] = {BK_JOURNAL_MAGIC, backup_crc}, took;
    const uint8_t *bytes = (const uint8_t *)&backup_bank;
    uint8_t readback[BK_BLOCK];
    if (fl_erase4k(BACKUP_JOURNAL_AT, &took) ||
        fl_write(BACKUP_JOURNAL_AT + sizeof header, bytes, sizeof backup_bank)) return -1;
    for (unsigned off = 0; off < sizeof backup_bank; off += sizeof readback) {
        unsigned n = sizeof backup_bank - off;
        if (n > sizeof readback) n = sizeof readback;
        if (st_read(BACKUP_JOURNAL_AT + sizeof header + off, readback, n) ||
            memcmp(readback, bytes + off, n)) return -1;
    }
    /* A valid header is written only after the whole bank is durable. */
    backup_restart_required = 1;
    return fl_write(BACKUP_JOURNAL_AT, (const uint8_t *)header, sizeof header) ||
        st_read(BACKUP_JOURNAL_AT, readback, sizeof header) ||
        memcmp(readback, header, sizeof header);
}

static void backup_recover(void)
{
    uint32_t header[2];
    if (!flash_ok || smk_store_cleared ||
        st_read(BACKUP_JOURNAL_AT, header, sizeof header) || header[0] != BK_JOURNAL_MAGIC) return;
    if (st_read(BACKUP_JOURNAL_AT + sizeof header, &backup_bank, sizeof backup_bank)) {
        backup_restart_required = 1;
        backup_commit_status = BK_FLASH;
        return;
    }
    /* If final erase was interrupted only the journal can be incomplete. */
    if (crc32((const uint8_t *)&backup_bank, sizeof backup_bank) != header[1] || !backup_valid()) return;
    if (backup_restore()) {
        backup_restart_required = 1;
        backup_committed_crc = header[1];
        backup_commit_status = BK_FLASH;
    }
}

static unsigned backup_execute(unsigned op, const uint8_t *args, unsigned size,
                               uint8_t *result, unsigned *result_size)
{
    uint8_t *bank = (uint8_t *)&backup_bank;
    unsigned off, count;
    if (op == BK_INFO && !size) {
        memcpy(result, "TRONBK1", 7);
        ota_wr16(result + 7, sizeof backup_bank);
        result[9] = backup_restart_required ? backup_commit_status == BK_OK ? 1u : 2u : 0u;
        ota_wr32(result + 10, backup_committed_crc);
        *result_size = 14;
        return BK_OK;
    }
    if (!flash_ok)
        return BK_FLASH;
    if ((int32_t)(fm1_ms - backup_deadline) >= 0)
        backup_mode = BK_NONE;

    switch (op) {
    case BK_EXPORT:
        if (size) return BK_INVALID;
        if (backup_restart_required) return BK_RESTART;
        backup_mode = BK_NONE;
        backup_snapshot();
        if (!backup_valid()) return BK_DATA;
        backup_mode = BK_READING;
        ota_wr32(result, crc32(bank, sizeof backup_bank));
        *result_size = 4;
        return BK_OK;
    case BK_READ:
        if (size != 2u) return BK_INVALID;
        if (backup_mode != BK_READING) return BK_EXPIRED;
        off = ota_rd16(args);
        if (off >= sizeof backup_bank) return BK_INVALID;
        count = sizeof backup_bank - off;
        if (count > BK_BLOCK) count = BK_BLOCK;
        memcpy(result, bank + off, count);
        *result_size = count;
        return BK_OK;
    case BK_UPLOAD:
        if (size != 4u) return BK_INVALID;
        if (backup_restart_required && (backup_commit_status == BK_OK || ota_rd32(args) != backup_committed_crc))
            return backup_commit_status == BK_OK ? BK_RESTART : BK_DATA;
        backup_crc = ota_rd32(args);
        backup_received = 0;
        backup_mode = BK_WRITING;
        return BK_OK;
    case BK_WRITE:
        if (size < 3u || size > BK_BLOCK + 2u) return BK_INVALID;
        if (backup_mode != BK_WRITING) return BK_EXPIRED;
        off = ota_rd16(args);
        count = size - 2u;
        if (off > backup_received || off + count > sizeof backup_bank) return BK_INVALID;
        if (off < backup_received)
            return off + count <= backup_received && !memcmp(bank + off, args + 2, count) ? BK_OK : BK_INVALID;
        memcpy(bank + off, args + 2, count);
        backup_received += count;
        return BK_OK;
    case BK_COMMIT:
        if (size != 4u) return BK_INVALID;
        /* A repeated commit after a lost reply is acknowledged without writing again. */
        if (backup_restart_required && backup_mode == BK_NONE && ota_rd32(args) == backup_committed_crc)
            return backup_commit_status;
        if (backup_mode != BK_WRITING) return BK_EXPIRED;
        if (backup_received != sizeof backup_bank || ota_rd32(args) != backup_crc ||
            crc32(bank, sizeof backup_bank) != backup_crc || !backup_valid()) return BK_DATA;
        if (!quiet() || seq_play || seq_rec || ch_rec || mm_on || pedal || fm1_in.keys ||
            ch_pads || fm1_in.buttons || save_req || ch_mix_dirty || scene_pending < 8u ||
            mm_pedal.candidate || platform_usb_recording()) return BK_BUSY;
        backup_mode = BK_NONE;
        backup_committed_crc = backup_crc;
        backup_commit_status = (backup_stage() || backup_restore()) ? BK_FLASH : BK_OK;
        if (backup_restart_required)
            backup_screen(backup_commit_status == BK_OK ? "RESTART TO PLAY" : "RESTART TO RETRY");
        return backup_commit_status;
    default:
        return BK_INVALID;
    }
}

static void backup_command(const uint8_t *body, unsigned size)
{
    if (size < 5u || ota_crc16(body, size - 2u, 0) != ota_rd16(body + size - 2u))
        return;
    unsigned count = 0;
    memcpy(backup_reply + 6, body, 3);
    backup_reply[9] = (uint8_t)backup_execute(body[0], body + 3, size - 5u, backup_reply + 10, &count);
    backup_deadline = fm1_ms + 15000u;
    unsigned length = count + 6u;
    ota_wr16(backup_reply + 10 + count, ota_crc16(backup_reply + 6, count + 4u, 0));
    backup_reply[0] = 0;
    backup_reply[1] = 0x59;
    backup_reply[2] = 0x63;
    ota_wr16(backup_reply + 3, length);
    backup_reply[5] = 0;
    unsigned sum = 0;
    for (unsigned i = 0; i < length; i++)
        sum += backup_reply[6 + i];
    backup_reply[6 + length] = (uint8_t)~sum;
    backup_wire[0] = 0xf0;
    unsigned end = 1u + ota_pack7(backup_reply, 7u + length, backup_wire + 1);
    backup_wire[end++] = 0xf7;
    ota_wire_send(backup_wire, end);
}
