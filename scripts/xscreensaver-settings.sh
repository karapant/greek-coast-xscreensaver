#!/usr/bin/env bash
set -euo pipefail
gc_project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
exec bash "$gc_project_dir/build/greek-coast-xscreensaver-settings" "$@"
