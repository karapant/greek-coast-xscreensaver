#!/usr/bin/env python3
"""Render moving geometry, night fishing light and animated terrace staff."""
from pathlib import Path
from PIL import Image,ImageChops,ImageStat
import os,subprocess,sys,tempfile
binary=Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='greek-coast-life-') as folder:
    root=Path(folder);env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1',XDG_CONFIG_HOME=str(root/'config'))
    def render(name,hour=12,phase=64,flags=(),width=640,height=360):
        path=root/(name+'.png')
        subprocess.run([str(binary),'--headless','--width',str(width),'--height',str(height),'--no-clouds','--date',f'2026-06-20T{hour:02d}:00:00+03:00','--time',str(phase),'--snapshot',str(path),*flags],env=env,check=True,capture_output=True)
        return Image.open(path).convert('RGB')
    def count(image,box,threshold=12):return sum(max(p)>threshold for p in image.crop(box).getdata())
    boat=render('boat',flags=('--no-taverna','--no-activity'))
    sea=render('sea',flags=('--no-taverna','--no-activity','--no-boat'))
    diff=ImageChops.difference(boat,sea)
    assert count(diff,(0,135,640,360))>200,'3D hull, mast, reflection and trailing wake must occupy visible pixels'
    assert count(diff,(0,0,640,130))==0,'boat cannot paint onto distant sky'
    boat2=render('boat2',phase=74,flags=('--no-taverna','--no-activity'))
    sea2=render('sea2',phase=74,flags=('--no-taverna','--no-activity','--no-boat'))
    diff2=ImageChops.difference(boat2,sea2)
    box1=diff.point(lambda x:255 if x>12 else 0).getbbox();box2=diff2.point(lambda x:255 if x>12 else 0).getbbox()
    assert box2[2]>box1[2]+20,'boat and wake travel across the water'
    night=render('night',hour=23,flags=('--no-taverna','--no-activity'))
    dark=render('dark',hour=23,flags=('--no-taverna','--no-activity','--no-boat'))
    lamp=ImageChops.difference(night,dark)
    assert count(lamp,(300,145,580,220),6)>80,'fishing lamp illuminates the nearby water at night'
    terrace=render('terrace',hour=23,phase=18,flags=('--no-boat',),width=1280,height=720)
    quiet=render('staff-hidden',hour=23,phase=18,flags=('--no-boat','--no-activity'),width=1280,height=720)
    hill=(376,195,483,232)
    staff=ImageChops.difference(terrace,quiet)
    assert count(staff,hill)>3,'small waiters must remain visible on the distant terrace'
    absent=render('taverna-hidden',hour=23,phase=18,flags=('--no-boat','--no-activity','--no-taverna'),width=1280,height=720)
    restaurant=ImageChops.difference(quiet,absent)
    assert count(restaurant,hill)>150,'taverna occupies a small part of the hillside village'
    assert count(restaurant,(0,290,1280,720),0)==0,'taverna and its patrons must never occupy foreground sea'
    bounds=restaurant.getbbox()
    assert bounds[0]>=375 and bounds[1]>=195 and bounds[2]<=483 and bounds[3]<=232,'all architecture stays attached to the hillside'
    day=render('day-terrace',flags=('--no-boat','--no-activity'),width=1280,height=720)
    assert sum(ImageStat.Stat(ImageChops.difference(day,quiet).crop(hill)).mean)>15,'daytime terrace blends to busy illuminated evening dining'
    children=render('children',phase=8,flags=('--no-boat',),width=1280,height=720)
    quietDay=render('children-hidden',phase=8,flags=('--no-boat','--no-activity'),width=1280,height=720)
    assert count(ImageChops.difference(children,quietDay),(395,214,425,224),6)>1,'children play at natural distant scale beside the taverna entrance'
    print('Moving 3D boat, wake, night fishing light and small hillside restaurant activity passed.')
