#!/usr/bin/env python3
"""Exercise registration against temporary homes, including a real multiline list."""
from pathlib import Path
import subprocess,sys,tempfile
script=Path(sys.argv[1]).resolve();binary=Path(sys.argv[2]).resolve()
with tempfile.TemporaryDirectory(prefix='greek-coast-register-') as directory:
    root=Path(directory)
    def run(home,*args):subprocess.run([sys.executable,str(script),'--home',str(home),'--binary',str(binary),*args],check=True,capture_output=True,text=True)
    for seed in ['', 'mode: one\nselected: 3\nlock: True\ntimeout: 0:10:00\nprograms: \\\n  GL: glmatrix --root \\n\\\n\nother: preserve\n']:
        home=root/str(len(seed));home.mkdir();config=home/'.xscreensaver'
        if seed:config.write_text(seed)
        run(home);first=config.read_text();run(home);second=config.read_text()
        assert first==second,'registration must be idempotent'
        assert second.count(str(binary))==1,'exactly one Greek Coast command'
        if seed:
            assert 'glmatrix --root' in second and 'timeout: 0:10:00' in second and 'other: preserve' in second
            assert (home/'.xscreensaver.before-greek-coast').read_text()==seed
        # Updating a legacy source-tree entry keeps the user's options.
        config.write_text(second.replace(str(binary),'/old checkout/build/greek-coast').replace('--speed 1','--speed 1000 --heading 267 --no-fish'))
        # Quote the command containing a space, as XScreenSaver expects.
        config.write_text(config.read_text().replace('/old checkout/build/greek-coast',"'/old checkout/build/greek-coast'"))
        run(home);updated=config.read_text();assert '--speed 1000 --heading 267 --no-fish' in updated
        assert updated.count(str(binary))==1 and '/old checkout' not in updated
        run(home,'--remove');removed=config.read_text();assert str(binary) not in removed
        if seed:assert 'glmatrix --root' in removed and 'lock: True' in removed
    if (script.parent/'greek-coast-xscreensaver-settings').is_file():
        home=root/'bridge';home.mkdir();config=home/'.xscreensaver'
        config.write_text('mode: random\ndemoCommand: custom-original-settings\nprograms: \\\n\n')
        run(home,'--settings-bridge');first=config.read_text();run(home,'--settings-bridge')
        assert first==config.read_text(),'bridge registration is idempotent'
        run(home,'--remove');assert 'demoCommand: custom-original-settings' in config.read_text(),'remove restores prior launcher preference'
        assert not (home/'.local/share/applications/xscreensaver-settings.desktop').exists(),'remove cleans its own managed launcher'
    print('Fresh registration, preserved preferences, idempotent updates, legacy migration and removal passed.')
