from pathlib import Path
import zipfile,json,hashlib,re
R=Path(__file__).resolve().parents[2];O=R/'dist/chords-preview'
files=[*R.glob('experiments/chords/*.py'),*R.glob('experiments/chords/*.h'),*R.glob('experiments/chords/*.txt'),*R.glob('templates/lab/*'),*R.glob('tests/*'),R/'README.md',R/'package.json',R/'dist/appearance-preview/mel.wasm']
files += [p for p in (R/'dist/manual').rglob('*') if p.is_file()]
files += list(R.glob('manual/*.py'))+list(R.glob('tools/*manual*'))
files += list(R.glob('experiments/chords/reference/*.wasm'))+list(R.glob('experiments/chords/reference/*.fwsc'))
files += [p for p in (R/'templates/installer').rglob('*') if p.is_file()]
files += [p for p in (R/'experiments/appearance/engine').rglob('*') if p.is_file() and 'build' not in p.relative_to(R/'experiments/appearance/engine').parts and '__pycache__' not in p.parts]
with zipfile.ZipFile(O/'source.zip','w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 for p in sorted(set(files)):z.write(p,p.relative_to(R).as_posix())
log=(R/'experiments/chords/native-build.txt').read_text();m=re.search(r'image (\d+) B of (\d+)',log);assert m
used,limit=map(int,m.groups())
manifest={'version':'0.3.0-ch4','official':'0.3.0-ch4','native_app_bytes':used,'native_limit_bytes':limit,'native_free_bytes':limit-used,'native_xip_overflow_bytes':max(0,used-limit),'installer':True,'sha256':{str(p.relative_to(R)):hashlib.sha256(p.read_bytes()).hexdigest() for p in [O/'mel.wasm',R/'dist/appearance-preview/mel.wasm',O/'source.zip']}}
(O/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
