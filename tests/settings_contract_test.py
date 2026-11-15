#!/usr/bin/env python3
"""Validate the native settings menu and its serialized command defaults."""
import json,os,shlex,subprocess,sys,tempfile
from pathlib import Path
import xml.etree.ElementTree as ET
root=ET.parse(sys.argv[1]).getroot();binary=str(Path(sys.argv[2]).resolve())
for group in root.findall('.//vgroup'):assert len(group)<=10,'native layout must stay compact'
speed=root.find('.//select[@id="speed"]');rates=[]
for option in speed:
    args=shlex.split(option.get('arg-set',''));rates.append(float(args[-1]) if args else 1.)
assert sorted(rates)==[.1,.5,1.,2.,5.,10.,25.,50.,100.,200.,500.,1000.]
for select in root.findall('.//select'):assert sum('arg-set' not in option.attrib for option in select)==1,'native dropdown requires a clear default'
assert root.find('.//boolean[@id="fish"]').get('arg-unset')=='--no-fish'
with tempfile.TemporaryDirectory(prefix='greek-coast-native-defaults-') as tmp:
    config=Path(tmp)/'greek-coast';config.mkdir();(config/'settings.ini').write_text('[cycle]\nmode=accelerated\nspeed=50\n[location]\nheading=90\n')
    env=dict(os.environ,XDG_CONFIG_HOME=tmp)
    result=subprocess.run([binary,'--root','--sky-info'],env=env,check=True,capture_output=True,text=True)
    state=json.loads(result.stdout)['settings'];assert state['speed']==1 and state['mode']=='real' and state['heading']==270,'native defaults must match visible menu labels'
print('Compact native menu, twelve speed presets, independent defaults and fish toggle passed.')
