#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Share identical/overlapping byte runs between glyphs, with no raster changes."""
import re
from pathlib import Path

def append_glyph(data, glyph):
    glyph=bytes(glyph)
    found=bytes(data).find(glyph)
    if found>=0:return found
    overlap=min(len(data),len(glyph))
    while overlap and bytes(data[-overlap:])!=glyph[:overlap]:overlap-=1
    offset=len(data)-overlap
    data.extend(glyph[overlap:])
    return offset

def compact_header(path):
    text=path.read_text();saved=0
    def array(name):
        return list(map(int,re.search(r'\b'+name+r'\[\] = \{(.*?)\};',text,re.S).group(1).replace('\n','').strip(',').split(',')))
    for name,h,last in re.findall(r'sp_font_t (SP_F\d+) = \{(\d+),(\d+),',text):
        widths=array(name+'_W');offsets=array(name+'_O');original=array(name+'_D');data=[];new_offsets=[]
        for w,at in zip(widths,offsets):
            glyph=original[at:at+(w*int(h)+3)//4]
            new_offsets.append(append_glyph(data,glyph))
        for w,at,new_at in zip(widths,offsets,new_offsets):
            n=(w*int(h)+3)//4
            assert original[at:at+n]==data[new_at:new_at+n],name
        saved+=len(original)-len(data)
        for suffix,values in [('O',new_offsets),('D',data)]:
            body='\n'+'\n'.join(','.join(map(str,values[i:i+24]))+',' for i in range(0,len(values),24))+'\n'
            text=re.sub(r'(\b'+name+'_'+suffix+r'\[\] = \{).*?(\};)',lambda m:m[1]+body+m[2],text,flags=re.S)
    path.write_text(text)
    print(f'Lossless glyph packing: {saved} bytes saved; every glyph verified byte for byte.')

if __name__=='__main__':
    import sys
    compact_header(Path(sys.argv[1]))
