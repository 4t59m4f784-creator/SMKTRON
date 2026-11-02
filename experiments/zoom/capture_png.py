from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import struct
R=Path(__file__).parent;P=R/'captures';D=R.parents[1]/'dist/zoom-preview/screens';D.mkdir(parents=True,exist_ok=True)
for p in P.glob('*.rgb565'):
 v=struct.unpack('<57600H',p.read_bytes());im=Image.new('RGB',(240,240));im.putdata([(((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31) for c in v]);im.save(D/(p.stem+'.png'))
board=Image.new('RGB',(1000,1060),'#0c1110');draw=ImageDraw.Draw(board);f=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',16)
for i,p in enumerate(['page-0','page-1','page-7','page-12']):
 x=12+(i%2)*494;y=12+(i//2)*524;draw.text((x,y),['PATCH','FLUTE + VIOLINS','REVERB','SYSTEM'][i],font=f,fill='#bfcfc6');board.paste(Image.open(D/(p+'.png')).resize((480,480),Image.Resampling.NEAREST),(x,y+28))
board.save(D.parent/'preview.png')
