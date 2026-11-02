"""Lossless 4bpp binary system fonts -> packed 1bpp. No smooth SP fonts changed."""
import re
def compact(engine):
    p=engine/'firmware/gen/felucca_font.h';s=p.read_text();records=[]
    def array(name):
     m=re.search(r'\b'+name+r'\[\d+\] = \{(.*?)\};',s,re.S)
     if not m:raise ValueError(name)
     return [int(x.strip(),0) for x in m[1].split(',') if x.strip()]
    def append(data,g):
     off=bytes(data).find(g)
     if off>=0:return off
     n=min(len(data),len(g))
     while n and data[-n:]!=g[:n]:n-=1
     off=len(data)-n;data.extend(g[n:]);return off
    for name,h,first,last in re.findall(r'felucca_font_t (FONT_[SL]) = \{\s*(\d+),\s*\d+,\s*(\d+),\s*(\d+),',s):
     h=int(h);raw=array(name+'_DATA');widths=array(name+'_BW');offsets=array(name+'_OFF');out=bytearray();new_offsets=[];pixels=0
     for w,off in zip(widths,offsets):
      row=(w+1)//2;packedrow=(w+7)//8;g=bytearray(h*packedrow)
      for y in range(h):
       for x in range(w):
        v=(raw[off+y*row+x//2]>>(0 if x&1 else 4))&15
        assert v in (0,15),(name,x,y,v) # No antialias level may be discarded.
        if v:g[y*packedrow+x//8]|=1<<(7-x%8)
      newoff=append(out,g);new_offsets.append(newoff)
      for y in range(h):
       for x in range(w):
        before=(raw[off+y*row+x//2]>>(0 if x&1 else 4))&15
        after=((out[newoff+y*packedrow+x//8]>>(7-x%8))&1)*15
        assert before==after,(name,x,y)
        pixels+=1
     assert len(out)<65536
     for suffix,vals,typ in [('DATA',list(out),'uint8_t'),('OFF',new_offsets,'uint16_t')]:
      body='\n'+'\n'.join(','.join(map(str,vals[i:i+24]))+',' for i in range(0,len(vals),24))+'\n'
      s=re.sub(r'static const '+typ+' '+name+'_'+suffix+r'\[\d+\] = \{.*?\};',f'static const {typ} {name}_{suffix}[{len(vals)}] = {{{body}}};',s,flags=re.S)
     records.append(dict(font=name,original_bytes=len(raw),packed_bytes=len(out),saved_bytes=len(raw)-len(out),verified_pixels=pixels))
    p.write_text(s)
    p=engine/'firmware/src/gfx.c';g=p.read_text();old='''        bpr = (w + 1u) / 2u;''';assert old in g;g=g.replace(old,'''        bpr = (w + 7u) / 8u;''')
    old='''                uint32_t v = gd[gy * bpr + gx / 2u];
                v = (gx & 1u) ? (v & 15u) : (v >> 4);''';assert old in g
    g=g.replace(old,'''                uint32_t v = ((gd[gy * bpr + gx / 8u] >> (7u - gx % 8u)) & 1u) * 15u;''');p.write_text(g)
    return records
