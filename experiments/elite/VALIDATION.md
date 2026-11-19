# Elite1 validation

Base: official ML1, commit a9a319cc58a6b1cb4c73230fa61126638b46a502. No physical Elite was connected or flashed. Installation, USB recovery and stock return remain hardware-untested.

- Elite compiler checks: app 603068 / 619520 bytes; static RAM 33712 / 98304; register access confined to HAL; RAM text contains no calls.
- Elite package: 651270 bytes; SHA-256 4e78c864ea788298c81b14ee6ed68d70b2d3249cc7094b91191b59201b9684f4. Product SMK-37 Elite_463, loader SMK-37 Elite_900, return record ota-SMK-37 Elite_016.
- Pinned official Elite V016 downloaded from the M-VAVE asset host and independently verified: 705236 bytes; SHA-256 b2ee38abfef3ae922e4eecdb8a551e14345300ae6f270265feacd33b6ef99cb6. Not committed or redistributed.
- smk_pkg_test.py: package CRCs, 20 markers, region table, app bounds and loader marker pass.
- ota_test.c compiled with BOARD_SMK37=1, T_MODEL=1: Elite identity, recovery, staged loader, return-record identity, disconnect and incomplete-session cleanup pass.
- ldr_test.c with BOARD_SMK37=1: simulated Elite V016 -> Elite1 install, repeat, damaged packets, lost host, unknown flash chip and protected ranges pass. The existing test harness labels its loader as Pro; production model matching is separately covered by the Elite app, package and browser-service tests.
- elite_stock_return_test.c with BOARD_SMK37=1, T_MODEL=1: actual firmware OTA code accepts the pinned official stock loader, commits Elite_016, rejects a foreign flash head. Does not execute the vendor loader on physical hardware.
- tests/elite-installer.test.mjs: 4 tests pass with ELITE_STOCK pointing to original V016, including actual package validation, wrong-model rejection, independent pending state, loader guards, no MIDI query for invalid buffers, repeated resume retaining the original source, stock return and page links.
- Official npm test: all 10 tests pass (simulator MIDI Layers, language, package/model pins, manual and desktop gestures).
- Pro rebuilt with the same toolchain/SDK: byte-for-byte SHA-256 8b975a1f38fb09727e697107603680325cb5bcca2a90807ef46d62e683a1b16f, identical to the official approved package. No official installer, homepage, simulator, release manifest or public release was changed.

Toolchain: jieli-linux-toolchains-20260730.1. SDK boot/config assets match all three SHA-256 pins in firmware/tools/build.py (AC79NN SDK V1.2.1). Rebuild with JIELI_TOOLCHAIN and AC79_SDK set:

```sh
python3 experiments/elite/build.py
python3 firmware/tests/smk_pkg_test.py firmware/build/mel-0.3.0-elite1-smk37-elite.fwsc 'SMK-37 Elite_463'
ELITE_STOCK=/path/to/Elite_016.fwsc node --test tests/elite-installer.test.mjs
npm test
```

Hardware references: https://github.com/genkigenki/fm1-to-smk37/blob/main/docs/01-hardware.md and https://genkigenki.github.io/smk37/help.html . Matching board support does not itself prove physical Elite compatibility.
