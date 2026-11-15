#!/usr/bin/env python3
"""Check rendered bodies, offscreen-source reflections and restrained activity."""
from pathlib import Path
import json
import os
import subprocess
import sys
import tempfile
from PIL import Image,ImageChops,ImageStat

binary=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='greek-coast-celestials-') as tmp:
    tmp=Path(tmp);env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1',XDG_CONFIG_HOME=str(tmp/'config'))
    def info(date,heading=270):
        return json.loads(subprocess.check_output([str(binary),'--sky-info','--date',date,'--heading',str(heading)],env=env,text=True))
    def render(name,date,phase=24,flags=(),heading=270,width=640,height=360):
        path=tmp/(name+'.png')
        subprocess.run([str(binary),'--headless','--no-boat','--no-taverna','--width',str(width),'--height',str(height),'--date',date,'--time',str(phase),'--heading',str(heading),'--snapshot',str(path),*flags],env=env,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        return Image.open(path).convert('RGB')
    def mean(image,box):return sum(ImageStat.Stat(image.crop(box)).mean)
    def projection(v):
        x,y,z=v
        return (int((.5+x/(-z)/(640/360*1.12))*640),int((1.-(.60+y/(-z)/1.05))*360))
    still=('--no-clouds','--no-activity')
    for body,date in [('sun','2026-06-21T17:10:00Z'),('moon','2026-11-21T00:00:00Z')]:
        sky=info(date);x,y=projection(sky[body]);assert 5<x<635 and 5<y<130
        a=render(body,date,flags=still);b=render(body+'-hidden',date,flags=(*still,'--no-celestials'))
        diff=ImageChops.difference(a,b)
        assert mean(diff,(x-4,y-4,x+5,y+5))>1,f'{body} texture is visible at its calculated sky location'
    date='2026-06-21T17:10:00Z'
    a=render('sun-behind',date,heading=90,flags=still)
    b=render('sun-behind-hidden',date,heading=90,flags=(*still,'--no-celestials'))
    assert mean(ImageChops.difference(a,b),(0,0,640,140))<.01,'body behind camera must not be forced into sky'
    # Moon is above the top of the frame, but still illuminates the water.
    date=next(d for d in ['2026-11-21T18:00:00Z','2026-11-21T19:00:00Z','2026-11-21T20:00:00Z']
              if info(d)['moon_elevation']>30 and info(d)['sun_elevation']< -10)
    a=render('offscreen-moon',date,flags=still)
    b=render('offscreen-moon-hidden',date,flags=(*still,'--no-celestials'))
    assert mean(ImageChops.difference(a,b),(0,200,640,360))>.02,'offscreen moon contributes directional light to water'
    date='2026-06-21T13:30:00+03:00'
    baseline=render('quiet',date,flags=still)
    cloudy=render('clouds',date,flags=('--no-activity',))
    clouds=ImageChops.difference(baseline,cloudy)
    assert mean(clouds,(0,0,640,125))>.05,'clouds appear in the sky'
    assert mean(clouds,(0,200,640,360))<.01,'cloud sprites cannot paint onto the sea'
    # Check naturally scaled distant actors at desktop resolution.
    baseline=render('village-quiet',date,flags=still,width=1280,height=720)
    active=render('activity',date,flags=('--no-clouds',),width=1280,height=720)
    activity=ImageChops.difference(baseline,active)
    assert mean(activity,(0,190,520,254))>0,'small villagers and vehicles appear on the village road'
    changed=sum(1 for pixel in activity.crop((0,190,520,254)).getdata() if max(pixel)>12)
    assert changed>=12,'village activity must cover visible pixels, not only subpixel differences'
    larger=render('larger-activity',date,flags=('--no-clouds','--activity-scale','2.5'),width=1280,height=720)
    largeDiff=ImageChops.difference(baseline,larger)
    largeCount=sum(1 for pixel in largeDiff.crop((0,190,520,254)).getdata() if max(pixel)>12)
    assert largeCount>changed,'activity size control makes people and cars more visible'
    assert mean(activity,(0,400,1280,720))<.01,'people and cars cannot paint onto the sea'
    later=render('later-clouds',date,phase=70,flags=('--no-activity',))
    assert mean(ImageChops.difference(cloudy,later),(0,0,640,125))>.05,'clouds drift on the natural animation clock'
    print('Celestial textures, offscreen reflections, clouds and village-road activity passed.')
