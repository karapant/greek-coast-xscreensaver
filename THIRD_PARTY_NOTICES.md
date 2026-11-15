# Third-party code and dependencies

Copies of the GNU LGPL/GPL license texts relevant to the optional system-library license choices are in `LICENSES/`; they do not change the project code from MIT.

Greek Coast code and generated artwork are offered under the MIT license in `LICENSE`. Original third-party notices remain authoritative. This audit covers the bundled source and directly linked dependencies; binary distributors must also account for libraries/plugins they bundle transitively.

| Component | Use | License / distribution notes |
| --- | --- | --- |
| PocketFFT | Bundled C++ header for spectral ocean transforms | BSD-3-Clause; retain its header notices and `vendor/pocketfft/LICENSE.md`, including with binaries. No FFTW library is linked. |
| Qt 6 Core, Gui, optional Widgets | Clock/configuration, matrices and standalone settings | These modules offer LGPL-3.0/GPL/commercial licensing. This build uses the LGPL option with dynamic linking. No GPL-only Qt modules are requested. Keep users able to replace the libraries; do not statically bundle Qt without reviewing LGPL requirements. |
| libepoxy | Dynamic OpenGL dispatch | MIT/Expat. Linked from system packages. |
| Xlib (libX11) | XScreenSaver window embedding | MIT-style X11 license. Linked from system packages. |
| EGL / Mesa or another system driver | Offscreen OpenGL and GPU runtime | Mesa is largely MIT; the actual driver/loader distribution has its own notices. No graphics driver is bundled. |
| libpng | PNG texture loading and snapshots | libpng license (permissive). Linked from system packages. |
| GTK 3, optional bridge only | Native dialog scrolling compatibility | LGPL-2.1-or-later. Dynamically linked; no GTK source is copied. |
| Python 3 / Pillow, tests only | Registration and image regression checks | Python PSF license / Pillow HPND-style license. No interpreter or Pillow package is bundled. |
| XScreenSaver | External host, not bundled | Its upstream permissive license remains separate. Greek Coast is an independent hack with its own XML; no XScreenSaver code is copied or modified on disk. |

PocketFFT's full copyright block is retained in the header, including the DCT algorithm contributors. Its BSD license permits this use; no GPL FFTW implementation was imported. Qt/GTK are the notable copyleft dependencies, with library replacement/source obligations when redistributing their binaries. Prefer distro-provided shared libraries. The license of Greek Coast does not relicense any dependency.

OpenSteer was reviewed as a steering-library option. Its project documentation describes separation, alignment and cohesion. This project implements its own small flocking simulation from the published Boids model; no OpenSteer source or binary is incorporated. See [Craig Reynolds' Boids page](https://www.red3d.com/cwr/boids/) and [OpenSteer documentation](https://opensteer.sourceforge.net/doc.html).

Upstream references: [Qt licensing](https://doc.qt.io/qt-6/licensing.html), [libepoxy](https://github.com/anholt/libepoxy), [libpng](http://www.libpng.org/pub/png/), [Mesa licensing](https://docs.mesa3d.org/license.html), [GTK](https://www.gtk.org/), [PocketFFT](https://github.com/mreineck/pocketfft). The installed distro copyright files were also checked during preparation.

PNG artwork was generated using OpenAI imagegen. Generation prompts are retained in `assets/GENERATION.md`; no third-party photos, fonts, logos or commercial texture packs are intentionally included. A copyright license can grant only rights that exist in the relevant jurisdiction; AI-generated artwork is not promised exclusive copyright protection. No OpenAI service, API key or image generation dependency is needed to run the screensaver.
