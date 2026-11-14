#!/usr/bin/env python3
"""Build the current manual from tracked editorial HTML and chapter sources."""
from pathlib import Path
import json,re,html
R=Path(__file__).resolve().parents[1]
s=(R/'manual/instrument.html').read_text()
s=s.replace('<!-- MIDI_MODE -->',(R/'manual/midi.html').read_text())
chapters=json.loads((R/'manual/index.json').read_text())
nav='<nav aria-label="Índice do manual">'+''.join(f'<a href="#{c["id"]}" data-nav="{c["id"]}"><span>{i+1:02}</span>{html.escape(c["title"])}</a>' for i,c in enumerate(chapters))+'</nav>'
s=re.sub(r'<nav aria-label="Índice do manual">.*?</nav>',nav,s,count=1,flags=re.S)
(R/'web/manual/index.html').write_text(s)
(R/'web/manual/index.json').write_text(json.dumps(chapters,ensure_ascii=False,indent=2)+'\n')
print(f'{len(chapters)} manual chapters written.')
