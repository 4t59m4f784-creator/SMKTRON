# SMKSTAGE B3 0.1.3-D4 CPU0 tonewheel diagnostic

*EXPERIMENTAL / NOT VALIDATED IN HARDWARE*. D4 is D3 with one engine dispatch change: all 91 tonewheels run on CPU0 using the existing full-step wheels_render(0,1) path; the CPU1 job dispatch macros are disabled only for this build. CPU1 may still initialize but handles no tonewheel jobs.

Other effects, UI, IRQ wrapper in RAM, keybed scan, presets, journal, flash safety and crash instrumentation are unchanged. Expect worse audio glitching under high load; if audio persistently breaks up, stop stress tests. The experiment isolates crashes from job handoff, not audio quality or performance.

GitHub source: https://github.com/4t59m4f784-creator/SMKSYNTH/tree/smkstage-b3-d4-singlecore-diag
Build: https://github.com/4t59m4f784-creator/SMKSYNTH/actions/runs/45337237855

**Pinned firmware**: SMK-37 Pro_19703, size 651339 bytes, SHA-256 e0ef655f4f02a9c1d0d966b280e995e213a99c96afc902e9facaf43f8b10b26a. Stock V016 and D4 are both required as local selected files. No automatic firmware download or write on connect. Only known SMK-37 Pro hardware.

If USB Rescue (_000): use ../install-011/ in the SAME browser profile with its known-good 0.1.1 fwsc and factory V016. Never retry D4 into Rescue. This installer blocks _000.
