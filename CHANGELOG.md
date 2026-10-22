# Changelog

## 0.3.0 - 2026-10-22

### Changed
- **Pick your consoles.** The consoles installer now opens on a box per system; only what you
  tick is built, so only those toolchains are downloaded, not all thirteen
  ([recomp-launcher 0.4.0](https://github.com/sp00nznet/recomp-launcher/releases/tag/v0.4.0)).
- The step every console shares (taking the sprites out of `ski32.exe`) runs in a 70 MB Alpine
  image instead of the 3.5 GB Emscripten one.

## 0.2.0 - 2026-10-22

SkiFree on 13 more systems, and the sound effects play.

### Added
- **Consoles, Android and the browser**, from an optional second installer,
  `SkiFree-Consoles-Installer` (or `Setup Consoles.cmd` / `setup-consoles.sh`). Built in Docker
  from your `ski32.exe`: PSP, PS Vita, PS2, PS3, PS4, Wii, GameCube, 3DS, original Xbox,
  Xbox 360, Dreamcast, Android and WebAssembly. See [docs/consoles.md](docs/consoles.md).
- The lite build (`SKIFREE_LITE`) these use: SDL2 alone, with no menus, no network game and an 8x8 font.
- `src/sdl2lite/`: a small software SDL2 for the PS3, Dreamcast and Xbox 360, whose toolchains have none.
- A controller's D-pad acts as the arrow keys, and a pad plugged in after start is picked up.

### Fixed
- **Sound effects never played**: the sounds were never embedded, and were only read from a mod
  folder. They're embedded now, and mixed on SDL's own audio, so SDL2_mixer is no longer needed.
- The left stick steered diagonally: it fed one axis into both X and Y.
- The status box locked its texture twice, and a sprite atlas could exceed a GPU's texture size.
- Sprite positions were read in a way that breaks on big-endian machines (Wii, GameCube, PS3, Xbox 360).

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
