#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
BOARD=smk37 MODEL=pro SMKTRON_STRINGS=1 SMKTRON_PLATE=1 SMKTRON_USB_STABLE=1 \
SMKTRON_MANUAL_SAVE=1 SMKTRON_FIXED_VELOCITY=1 SMKTRON_PAD_PALETTE=1 \
SMKTRON_REFINEMENTS=1 SMKTRON_FLUTE_BALANCE=1 SMKTRON_SCENES=1 \
    sh firmware/build.sh --release 0.3.0-bk2
cp firmware/build/mel-0.3.0-bk2-smk37.fwsc web/backup/
python3 - <<'PY'
import hashlib
import json
from pathlib import Path

package = Path("web/backup/mel-0.3.0-bk2-smk37.fwsc")
data = package.read_bytes()
metadata = {
    "name": "SMKTRON 0.3.0-bk2",
    "product": "SMK-37 Pro_465",
    "url": "./" + package.name,
    "size": len(data),
    "sha": hashlib.sha256(data).hexdigest(),
}
package.with_name("firmware.js").write_text(
    "export const preview = " + json.dumps(metadata, indent=4) + ";\n"
)
PY
