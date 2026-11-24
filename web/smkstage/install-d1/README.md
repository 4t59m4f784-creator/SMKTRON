# SMKSTAGE B3 hardware forensic diagnostic 0.1.1-D1

ONLY firmware SHA-256 `61372501e13b6fed9fa24febad26ac4effc95266c4e2fca40b903831be3e7887`, identity `SMK-37 Pro_19703`, size 651339 bytes.

This isolated installer is not a public release; it uses exactly the same strict USB SysEx protocol and known-device allowlist as SMKSTAGE 0.1.1. It does NOT accept USB Rescue identity _000. To recover after a D1 failure, use `../install-011/` in the same browser; that page can validate the D1 failed-transaction record and install pinned proven-playable 0.1.1.

Source: https://github.com/4t59m4f784-creator/SMKSYNTH/tree/smkstage-b3-crash-diag-011
Build workflow: https://github.com/4t59m4f784-creator/SMKSYNTH/actions/runs/45328972724

Experimental, first real device test not complete. No change to the official SMKSYNTH/SMKTRON installers.
