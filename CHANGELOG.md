# Changelog

## 0.1.0 - 2026-10-22

The first release, and the first with an installer.

### Added
- **Linux and macOS.** The same game, built natively with SDL2. The menus drop down from the top of the
  window, since there is no native menu bar off Windows; Save and Load use `skifree_save.json`.
- **Installers for Windows, Linux and macOS** ([recomp-launcher](https://github.com/sp00nznet/recomp-launcher)):
  choose the folder holding `ski32.exe`, Build, Play. Built by CI on every push and attached to `v*` releases.
- **One-step setup**: `Setup.cmd` on Windows (installs Python, CMake and the Visual Studio Build Tools with
  winget if missing, and fetches SDL2's own prebuilt libraries), `./setup.sh` on Linux and macOS.
- CI builds and starts the game on all three systems.

### Changed
- `tools/extract_resources.py` needs only Python's standard library (no `pefile`) and takes the sounds from
  `ski32sounds.exe` too.
- SDL2_image and the cJSON package are no longer needed: sprites load with SDL's own BMP reader, and cJSON
  is in `third_party/`.
- The status window falls back to ProggyClean when Windows' `vgaoem.fon` isn't there.

### Fixed
- The game no longer crashes on a machine with no GPU renderer: SDL's software renderer takes over.
