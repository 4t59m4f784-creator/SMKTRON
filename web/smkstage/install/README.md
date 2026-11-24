# SMKSTAGE B3 Alpha — Web Installer

Web URL: https://4t59m4f784-creator.github.io/SMKTRON/smkstage/install/

Temporary home in a separate `web/smkstage/install` folder. No changes to the existing SMKTRON/SMKSYNTH installers or public firmware.

**Development build, not hardware tested**: requires choosing BOTH the pinned experimental .fwsc (`SMK-37 Pro_19703`, SHA-256 `2d992f98864558b24d61a007e20c7c6c192a441489c5be4c90c2ca20881cd97b`) and the original M-VAVE V016 .fwsc. No firmware payload is automatically downloaded. The Alpha zip is in GitHub Actions run [45323049939](https://github.com/4t59m4f784-creator/SMKSYNTH/actions/runs/45323049939) and currently expires according to Actions artifact retention.

Protection: SHA-256 + size + identity + JieLi container CRC, strict SMK-37 Pro model and firmware origin allowlist (V016 / SMKSYNTH 19668 / SMKTRON 463 / SMKSTAGE 19703), exact known OTA 900 loader with pending-operation binding, read-only identification, required consent and typed word, final confirmation, browser lock, log export, and post-boot identity verification. Source code for this installer is GPL-3.0-only, retaining original credits in package/protocol modules.

The **Alpha firmware has not been tested on the physical keyboard**, therefore final installation/recovery safety remains unverified. Do not present this as official software.

Transition to stable SMKSTAGE: create dedicated repo, pin release assets, package first install and update separately if warranted, verify flash/recovery and hardware test before enabling public distribution.
