# SMKSTAGE B3 Alpha — Web Installer

Web URL: https://4t59m4f784-creator.github.io/SMKTRON/smkstage/install/

Temporary home in a separate `web/smkstage/install` folder. No changes to the existing SMKTRON/SMKSYNTH installers or public firmware.

**Development build, not hardware tested**: requires choosing BOTH the pinned experimental .fwsc (`SMK-37 Pro_19703`, SHA-256 `a31c8014f087aada9d0880440acc0c4ba7b907c5489afa1c96b5d4862ec5db68`) and the original M-VAVE V016 .fwsc. No firmware payload is automatically downloaded. The Alpha zip is in GitHub Actions run [45207502485](https://github.com/4t59m4f784-creator/SMKSYNTH/actions/runs/45207502485) and currently expires according to Actions artifact retention.

Protection: SHA-256 + size + identity + JieLi container CRC, strict SMK-37 Pro model and firmware origin allowlist (V016 / SMKSYNTH 19668 / SMKTRON 463 / SMKSTAGE 19703), exact known OTA 900 loader with pending-operation binding, read-only identification, required consent and typed word, final confirmation, browser lock, log export, and post-boot identity verification. Source code for this installer is GPL-3.0-only, retaining original credits in package/protocol modules.

The **Alpha firmware has not been tested on the physical keyboard**, therefore final installation/recovery safety remains unverified. Do not present this as official software.

Transition to stable SMKSTAGE: create dedicated repo, pin release assets, package first install and update separately if warranted, verify flash/recovery and hardware test before enabling public distribution.
