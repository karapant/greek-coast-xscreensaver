# Development notes

The renderer requires desktop OpenGL 3.3. Headless snapshots use surfaceless EGL. The ocean uses a Phillips spectrum and the bundled PocketFFT inverse transform. Clouds are ray-marched bounded volumes; village actors and fish use generated texture artwork. Boats are actual meshes with multisampled direct/reflection passes. Fish and idle people share a runtime atlas to stay within the minimum fragment texture-unit budget.

People and fish run at fixed 30 Hz simulation steps independent of rendering and astronomical acceleration. People follow authored paths and tasks; fish use separation, alignment and cohesion. The camera remains at a fixed photographic viewpoint. Boat paths are separated in depth and time, and the active vessel changes only after it has passed offscreen.

Build-tree execution uses copied runtime data in `build/share/greek-coast`. Reconfigure after editing assets. Installed data is resolved relative to the executable; `DESTDIR` staging, relocation and manifest-based uninstall are tested. The normal install does not modify desktop launchers. The optional settings bridge handles user-local XML lookup and scrolling and removes its own preload token before spawning preview subprocesses.

The native preview recovery test covers direct host rendering, remapping, fullscreen obscuring, SIGSTOP/SIGCONT, ancestor remapping and resize. The renderer rebinds GLX drawing/read buffers after return, retries transient drawable loss while the host remains alive, and ends when its host is destroyed. These tests require a real or virtual X server; offscreen snapshots cannot establish that the actual desktop Escape flow works. Check that interaction on a desktop before publishing a release.

References: [NOAA solar equations](https://www.gml.noaa.gov/grad/solcalc/solareqns.PDF), [Schlyter lunar model](https://stjarnhimlen.se/comp/ppcomp.html), [Reynolds Boids](https://www.red3d.com/cwr/boids/), [OpenSteer](https://opensteer.sourceforge.net/). No OpenSteer code is included. Generated asset prompts are in [GENERATION.md](../assets/GENERATION.md).
