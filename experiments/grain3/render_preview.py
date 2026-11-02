from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import struct,json
R=Path(__file__).resolve().parents[2];C=R/'experiments/grain3/captures';D=R/'dist/grain3-preview'
font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',16)
def load(p):
 v=struct.unpack('<57600H',p.read_bytes());im=Image.new('RGB',(240,240));im.putdata([(((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31) for c in v]);return im
frames=[]
for i in range(30):
 board=Image.new('RGB',(1112,452),'#0c1411');draw=ImageDraw.Draw(board)
 for t,label in enumerate(['LIGHT','DARK','ZOOM']):
  x=12+t*368;draw.text((x,8),label,font=font,fill='#cad5ce');board.paste(load(C/f'grain-{t}-{i}.rgb565').resize((352,352),Image.Resampling.NEAREST),(x,38))
  c=tuple(json.loads((C/f'grain-{t}-{i}.json').read_text()));draw.text((x,410),'PAD 10',font=font,fill='#cad5ce');draw.rounded_rectangle((x+100,402,x+158,436),radius=7,fill=c,outline='#394e45')
 frames.append(board)
frames[0].save(D/'preview.png')
frames[0].save(D/'animation.gif',save_all=True,append_images=frames[1:],duration=80,loop=0,optimize=True,disposal=2)
for t in range(3):load(C/f'grain-{t}-3.rgb565').resize((720,720),Image.Resampling.NEAREST).save(D/f'grain-{t}.png')
print('Generated GIF from actual firmware framebuffers and LED RGB values.')
