# Contributing

Build with CMake and run CTest before submitting a change. Use `-DGREEK_COAST_SETTINGS_BRIDGE=ON` to exercise the optional GTK integration. Keep GTK-specific code separate from the normal renderer and avoid source-tree paths in installed files.

Before a release:

1. Build and test a clean default checkout and the optional bridge configuration.
2. Stage installation with `DESTDIR`, verify installed rendering after moving the prefix, and test registration/update/removal against a temporary home directory.
3. Exercise native Settings and preview return on a real X11 desktop (or Xvfb for basic GUI tests).
4. Verify source packages exclude build files, credentials, personal preferences and generated diagnostic exports.
5. Preserve PocketFFT notices and update `THIRD_PARTY_NOTICES.md` whenever dependencies change.

Use MIT-compatible contributions. Preserve all third-party license notices. Do not add unlicensed photos or texture packs. Keep generated asset prompts in `assets/GENERATION.md`; runtime users should never need a generation service or API key.

No remote repository, release, commit or tag is created by the build or installer.
