"""Stereo tape/TONE/EQ front end, one shared modulation/echo/reverb chain.
Normal mode executes the original mono path exactly; no duplicate delay tanks.
"""
import re

def stereo_fx(s):
 fields='lg lg2 hg hg2 lp1 lp2 held lc dc_x dc_y dlp d1 d2 ls hs sv_lp sv_bp'.split()
 a=s.index('    static int32_t lg, lg2,');b=s.index('#if !SMKTRON_PLATE',a)
 s=s[:a]+'''    typedef struct {int32_t '''+', '.join(fields)+''';} tape_state_t;
    static tape_state_t tape[2];
    static int32_t hiss,hiss2,drop_g=32768,am,vt,les_x,e_lp,e_d=8000<<8;
'''+s[b:]
 s=s.replace('static void fx_run(const int32_t *mix, int32_t *out, uint32_t n, int32_t act)', 'static void fx_run(const int32_t *mix, const int32_t *right, int32_t *out, uint32_t n, int32_t act)')
 s=s.replace('int32_t x = mix[i], y, y0, l, r, in, dry = 0, wetl = 32767 - bl, wetf = 32767 - bf;', 'int32_t x,y,y0l,y0r,directl,directr,l,r,in,dry,wetl=32767-bl,wetf=32767-bf,chan_y[2];\n        uint32_t channel;')
 a=s.index('        x = x > 65535');b=s.index('        /* the pedals (FX) */',a)
 front=s[a:b]
 front=front.replace('        bl += (blt - bl) >> 8;                  /* the bypasses, ramped (~6 ms) */\n        bf += (bft - bf) >> 8;\n','')
 front=front.replace("            am += (amt - am) >> 6;              /* FLUTTER's roughness */\n",'')
 front=front.replace('            drop_g += (drop_t - drop_g) >> 7;   /* DROPOUT */\n','')
 rng=front.index('            rnd2 ^= rnd2 << 13;');end=front.index('            y += (hiss2 * hl)',rng)
 noise=front[rng:end];front=front[:rng]+front[end:]
 for field in sorted(fields,key=len,reverse=True):front=re.sub(r'\b'+field+r'\b','ts->'+field,front)
 prefix='''        bl += (blt-bl)>>8; bf += (bft-bf)>>8;
        if(!(bl>=32640 && blt)){
            am+=(amt-am)>>6;drop_g+=(drop_t-drop_g)>>7;
'''+noise+'''        }
        for(channel=0;channel<(right?2u:1u);channel++){
            tape_state_t *ts=&tape[channel];
            x=channel?right[i]:mix[i];dry=0;
'''
 s=s[:a]+prefix+front+'''            chan_y[channel]=y;
        }
        y0l=chan_y[0];y0r=right?chan_y[1]:y0l;
        directl=y0l;directr=y0r;y=(y0l+y0r)>>1;
'''+s[b:]
 s=s.replace('        y0 = y;\n','')
 s=s.replace('            l = r = y;\n            goto trails;', '            l=y0l;r=y0r;\n            goto trails;')
 s=s.replace('            y += ((vt - y) * vmix) >> 15;', '            y += ((vt - y) * vmix) >> 15;\n            directl += ((vt-directl)*vmix)>>15;directr += ((vt-directr)*vmix)>>15;')
 s=s.replace('            l = y + (((l - y) * lmix) >> 15);', '            l = directl + (((l - directl) * lmix) >> 15);')
 s=s.replace('            r = y + (((r - y) * lmix) >> 15);', '            r = directr + (((r - directr) * lmix) >> 15);')
 s=s.replace('            l = r = y;', '            l=directl;r=directr;')
 s=s.replace('l = y0 + (((l - y0)', 'l = y0l + (((l - y0l)').replace('r = y0 + (((r - y0)', 'r = y0r + (((r - y0r)')
 s=s.replace('    lofi_bp = bl;', '    if(!right)tape[1]=tape[0];\n    lofi_bp = bl;')
 return s
