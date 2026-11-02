"""Convert captured framebuffer bytes to native-resolution screenshots, losslessly."""
from pathlib import Path
from PIL import Image
import struct
R=Path(__file__).resolve().parents[1]
for p in (R/'manual/capture-work').glob('*.rgb565'):
    pixels=struct.unpack('<57600H',p.read_bytes())
    im=Image.new('RGB',(240,240))
    im.putdata([(((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31) for c in pixels])
    im.save(R/'dist/manual/images'/(p.stem+'.png'))
