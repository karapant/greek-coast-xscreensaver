#!/usr/bin/env python3
from PIL import Image,ImageChops
from pathlib import Path
import os,subprocess,sys,tempfile
binary=str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix='greek-coast-fish-') as tmp:
    env=dict(os.environ,LIBGL_ALWAYS_SOFTWARE='1',XDG_CONFIG_HOME=tmp)
    def render(name,phase=8,off=False):
        file=Path(tmp)/(name+'.png')
        args=[binary,'--headless','--width','1280','--height','720','--no-boat','--no-clouds','--date','2026-06-20T12:00:00+03:00','--time',str(phase),'--snapshot',str(file)]
        if off:args+=['--no-fish']
        subprocess.run(args,env=env,check=True,capture_output=True);return Image.open(file).convert('RGB')
    on=render('on');off=render('off',off=True);difference=ImageChops.difference(on,off)
    count=sum(max(p)>12 for p in difference.getdata())
    assert count>120,('shoals must be discernible without enlarging fish into foreground objects',count)
    assert difference.crop((0,0,1280,330)).getbbox() is None,'fish must remain underwater, never over sky or village'
    later=render('later',phase=14);quiet=render('later-off',phase=14,off=True)
    assert ImageChops.difference(difference,ImageChops.difference(later,quiet)).getbbox(),'shoals and tails must move'
    print('Visible small shoals, underwater-only placement, movement and menu toggle passed.')
