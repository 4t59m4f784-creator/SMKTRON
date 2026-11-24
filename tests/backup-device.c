/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#define DINGHY_WEB 1
#define BOARD_SMK37 1
#define SMKTRON_SCENES 1
#define SMKTRON_STRINGS 1
#define SMKTRON_USB_STABLE 1
#define SMKTRON_PLATE 1
#define SMKTRON_PAD_PALETTE 1
#define SMKTRON_REFINEMENTS 1
#define SMKTRON_MANUAL_SAVE 1
#define SMKTRON_BACKUP 1
#define FELUCCA_ID "SMK-37 Pro_464"
#include "../firmware/firmware/src/mel.c"

static int flash_ok = 1;
static uint32_t crc32(const uint8_t *p, uint32_t n) { return ~sc_crc(~0u, p, n); }
static int ota_wire_send(const uint8_t *p, uint32_t n)
{
    for (unsigned i = 0; i < n; i++)
        printf("%02x", p[i]);
    putchar('\n');
    fflush(stdout);
    return 0;
}
static uint8_t input_frame[640];
static uint32_t input_size;
static int ota_frame_get(const uint8_t **p, uint32_t *n)
{
    *p = input_frame;
    *n = input_size;
    return input_size != 0;
}
static void ota_frame_done(void) { input_size = 0; }
static uint32_t ota_now_ms(void) { return fm1_ms; }
static void ota_idle(void) {}
static int ota_erase(uint32_t off) { (void)off; return -1; }
static int ota_prog(uint32_t off, const void *p, uint32_t n) { (void)off; (void)p; (void)n; return -1; }
static int ota_fread(uint32_t off, void *p, uint32_t n) { (void)off; (void)p; (void)n; return -1; }
static void ota_show(uint32_t step, int32_t code) { (void)step; (void)code; }
static void ota_commit(const uint8_t *parm) { (void)parm; }

#include "../firmware/firmware/src/ota.c"
#include "../firmware/firmware/src/smktron_backup.h"

int main(void)
{
    app_init();
    sc_save(3);
    ch_bank_t chords = ch_bank;
    chords.count[2] = 3;
    chords.notes[2][0] = 60;
    chords.notes[2][1] = 64;
    chords.notes[2][2] = 67;
    ch_commit(&chords);
    char line[1400];
    while (fgets(line, sizeof line, stdin)) {
        if (!memcmp(line, "fail\n", 6) || !memcmp(line, "recover\n", 9)) {
            ch_store_fail_after = line[0] == 'f' ? 0u : ~0u;
            puts("ok");
            fflush(stdout);
            continue;
        }
        if (!memcmp(line, "reboot\n", 8)) {
            app_init();
            backup_mode = backup_restart_required = backup_received = 0;
            puts("ok");
            fflush(stdout);
            continue;
        }
        uint8_t wire[640];
        unsigned n = 0, byte;
        while (n < sizeof wire && sscanf(line + n * 2, "%2x", &byte) == 1) {
            wire[n++] = (uint8_t)byte;
            if (byte == 0xf7u) break;
        }
        if (n < 2 || wire[0] != 0xf0 || wire[n - 1] != 0xf7) return 1;
        input_size = n - 2;
        memcpy(input_frame, wire + 1, input_size);
        ota_service();
    }
    return 0;
}
