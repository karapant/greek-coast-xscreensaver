#!/usr/bin/env python3
"""Stage installation, relocate its prefix, render, register/remove and uninstall."""
from pathlib import Path
import os,shutil,subprocess,sys,tempfile
build=Path(sys.argv[1]).resolve()
cache=(build/'CMakeCache.txt').read_text().splitlines()
prefix=Path(next(line.split('=',1)[1] for line in cache if line.startswith('CMAKE_INSTALL_PREFIX:')))
with tempfile.TemporaryDirectory(prefix='greek-coast-install-') as folder:
    root=Path(folder);stage=root/'stage';env=dict(os.environ,DESTDIR=str(stage),LIBGL_ALWAYS_SOFTWARE='1',XDG_CONFIG_HOME=str(root/'config'))
    subprocess.run(['cmake','--install',str(build)],env=env,check=True,capture_output=True)
    installed=stage/str(prefix).lstrip('/')
    assert installed.is_dir(),installed
    candidates=[p for p in installed.rglob('greek-coast') if p.is_file() and os.access(p,os.X_OK)]
    assert len(candidates)==1,candidates
    relative=candidates[0].relative_to(installed)
    relocated=root/'relocated prefix with spaces';shutil.copytree(installed,relocated)
    binary=relocated/relative
    subprocess.run([str(binary),'--headless','--width','320','--height','180','--frames','1','--cycle','accelerated','--speed','1'],env=env,check=True,capture_output=True)
    register=next(relocated.rglob('greek-coast-register'));home=root/'home';home.mkdir()
    subprocess.run([sys.executable,str(register),'--home',str(home)],env=env,check=True,capture_output=True)
    assert str(binary) in (home/'.xscreensaver').read_text(),'relocated registration points at installed executable'
    subprocess.run([sys.executable,str(register),'--home',str(home),'--remove'],env=env,check=True,capture_output=True)
    assert str(binary) not in (home/'.xscreensaver').read_text()
    xmls=list(stage.rglob('greek_coast.xml'));assert xmls and xmls[0].read_text()==xmls[0].with_name('greek-coast.xml').read_text(),'native title alias is installed'
    notices=list(relocated.rglob('THIRD_PARTY_NOTICES.md'));assert notices,'license notices installed'
    manifest=(build/'install_manifest.txt').read_text().splitlines()
    subprocess.run(['cmake','-P',str(build/'uninstall.cmake')],env=env,check=True,capture_output=True)
    assert all(not (stage/path.lstrip('/')).exists() for path in manifest),'uninstall removes only the staged manifest paths'
    print('DESTDIR installation, relocation including spaces, installed rendering, registration/removal and manifest uninstall passed.')
