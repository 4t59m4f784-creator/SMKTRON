"""Lossless shared glyph pool for the existing Terminus display fonts."""
import re

def pack(R):
 p=R/'firmware/gen/mel_fonts.h';s=p.read_text();pool=bytearray();records=[];total=0
 def append(g):
  at=pool.find(g)
  if at>=0:return at
  n=min(len(pool),len(g))
  while n and pool[-n:]!=g[:n]:n-=1
  at=len(pool)-n;pool.extend(g[n:]);return at
 for name,w,h,first,last in re.findall(r'mel_font_t (F\d+) = \{(\d+), (\d+), (\d+), (\d+),',s):
  m=re.search(r'static const uint8_t '+name+r'_DATA\[\d+\] = \{(.*?)\};',s,re.S)
  data=bytes(int(x.strip(),0) for x in m[1].split(',') if x.strip());n=((int(w)+7)//8)*int(h);offsets=[];total+=len(data)
  for i in range(int(last)-int(first)+1):
   glyph=data[i*n:(i+1)*n];o=append(glyph);offsets.append(o)
   assert bytes(pool[o:o+n])==glyph,(name,i)
  records.append((name,w,h,first,last,offsets,m[0]))
 s=s.replace('const uint8_t *data;','const uint8_t *data; const uint16_t *off;')
 for name,w,h,first,last,offsets,old in records:
  s=s.replace(old,'static const uint16_t '+name+'_OFF[] = {'+','.join(map(str,offsets))+'};')
  s=s.replace(name+'_DATA};','MEL_GLYPHS,'+name+'_OFF};')
 pos=s.index('static const uint16_t')
 s=s[:pos]+'static const uint8_t MEL_GLYPHS[] = {'+','.join(map(str,pool))+'};\n'+s[pos:]
 p.write_text(s)
 p=R/'firmware/src/mel_ui.c';s=p.read_text();a='gd = f->data + (ch - f->first) * bpr * f->h;';assert a in s;s=s.replace(a,'gd = f->data + f->off[ch - f->first];');p.write_text(s)
 print('Lossless Terminus pool:',total,'->',len(pool)+sum(len(r[5])*2 for r in records),'bytes; every glyph verified.')
