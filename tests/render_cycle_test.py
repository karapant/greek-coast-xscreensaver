#!/usr/bin/env python3
"""Exercise actual shaders across lighting states, not a duplicate CPU model."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile
from PIL import Image, ImageChops, ImageStat

binary=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='greek-coast-lighting-') as tmp:
    tmp=Path(tmp)
    env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1',XDG_CONFIG_HOME=str(tmp/'config'))
    def render(name,date,phase=24):
        path=tmp/(name+'.png')
        subprocess.run([str(binary),'--headless','--no-boat','--no-taverna','--no-clouds','--no-activity','--width','640','--height','360','--date',date,'--time',str(phase),'--snapshot',str(path)],env=env,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        return Image.open(path).convert('RGB')
    day=render('day','2026-06-21T13:30:00+03:00')
    night=render('night','2026-06-21T23:00:00+03:00')
    moving=render('moving','2026-06-21T23:00:00+03:00',25)
    dusk=render('dusk','2026-06-21T20:55:00+03:00')
    def brightness(image,box):return sum(ImageStat.Stat(image.crop(box)).mean)
    sky=(0,0,640,65);sea=(0,200,640,360)
    assert brightness(day,sky)>brightness(dusk,sky)>brightness(night,sky),'sky transitions from day through dusk to night'
    assert brightness(night,sea)<brightness(day,sea)*.3,'water darkens with the sky'
    village=night.crop((0,85,235,140))
    warm=sum(1 for r,g,b in village.getdata() if r>70 and r>b*1.5 and g>b*1.1)
    assert warm>30,'night buildings and roads have warm practical lights'
    delta=ImageChops.difference(night,moving)
    assert brightness(delta,sky)==0,'sky does not move with wave phase'
    assert brightness(delta,sea)>.2,'night water and light reflections animate'
    print('Actual day/dusk/night shaders, practical lights, and animated water passed.')
