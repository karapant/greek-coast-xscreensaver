#!/usr/bin/env python3
"""XScreenSaver decimal arguments must survive Greek and C desktop locales."""
import json,locale,os,subprocess,sys,tempfile
from pathlib import Path
binary=str(Path(sys.argv[1]).resolve())
locales=['C.UTF-8']
try:
    locale.setlocale(locale.LC_NUMERIC,'el_GR.utf8');locales.append('el_GR.utf8')
except locale.Error:
    print('Greek locale unavailable; exercising strict decimal parsing in C locale.')
with tempfile.TemporaryDirectory(prefix='greek-coast-numeric-') as folder:
    for language in locales:
        env=dict(os.environ,LC_ALL=language,XDG_CONFIG_HOME=folder)
        for text,speed in [('0.1',.1),('1',1.),('1.0',1.),('1000.0',1000.),('0,1',.1),('1,0',1.)]:
            command=[binary,'--sky-info','--cycle','accelerated','--speed',text,'--heading','267.5','--activity-scale','1.75','--wind','5.25','--time','24.5']
            run=subprocess.run(command,env=env,capture_output=True,text=True)
            assert run.returncode==0,(language,text,run.stderr)
            settings=json.loads(run.stdout)['settings']
            assert settings==dict(mode='accelerated',speed=speed,heading=267.5,activity_scale=1.75,wind=5.25,time=24.5),(language,settings)
        for text in ['0','1001','1x','0.1junk','nan','inf','1,2,3']:
            run=subprocess.run([binary,'--sky-info','--speed',text],env=env,capture_output=True,text=True)
            assert run.returncode==1,(language,text,'invalid controls must fail cleanly, not crash or be silently truncated')
    print('Greek/C locale round trips for 0.1x, 1x, 1000x, decimal controls and strict invalid-input rejection passed.')
