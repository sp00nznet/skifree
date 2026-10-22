# SkiFree on consoles, Android and the browser

The **SkiFree Consoles Installer** (or `Setup Consoles.cmd` / `./setup-consoles.sh`)
builds every target below from your own `ski32.exe`, inside Docker. Each lands in
`builds/<target>/`, and `builds/README.txt` says what built.

These builds are the *lite* game: the original SkiFree with a controller, but no
menus, no network game and an 8x8 font in the status box. The yeti is unchanged.

| Target | File | Last seen running |
|---|---|---|
| Browser | `wasm/skifree.html` (+ `.js`, `.wasm`) | Chrome: title screen, sounds load |
| Android | `android/skifree.apk` (arm64 and x86_64) | Android emulator: touch steers |
| PSP | `psp/EBOOT.PBP` | PPSSPP: title screen |
| PS Vita | `vita/skifree.vpk` | Vita3K: title screen, 61 fps |
| Nintendo 3DS | `3ds/skifree.3dsx` | Azahar: title screen, 60 fps |
| Wii | `wii/skifree.dol` | Dolphin: title screen |
| GameCube | `gamecube/skifree.dol` | Dolphin: title screen |
| PlayStation 2 | `ps2/skifree.elf` | PCSX2: title screen, 60 fps |
| PlayStation 3 | `ps3/EBOOT.BIN` | RPCS3: title screen, ~60 fps |
| PlayStation 4 | `ps4/skifree.pkg` | Builds; no emulator to try it in |
| Original Xbox | `xbox/skifree.xbe`, `skifree.iso` | xemu: title screen |
| Xbox 360 | `xenon/skifree.elf32` | Builds; untested (no emulator runs libxenon homebrew) |
| Dreamcast | `dreamcast/skifree.elf`, `skifree.cdi` | Flycast: title screen |

"Title screen" means the game started, drew everything and found the pad. Nobody
has skied a full run on real hardware yet: tell us if you do.

## Controls

| | |
|---|---|
| Left stick | steer (points the skier like the mouse did) |
| D-pad | the arrow keys |
| A / Cross | jump |
| Start | pause |
| Back / Select | restart (Y on the Dreamcast) |
| Right bumper / R1 | turbo: outrun the yeti |

On Android, hold your finger where you want to go and tap to jump.

## Installing each

- **Browser:** serve the three files from any web server (`python3 -m http.server`
  in the folder) and open `skifree.html`. Opening it as a file doesn't work.
- **Android:** copy the APK to the phone and open it (allow installs from that app).
- **PSP:** `PSP/GAME/SKIFREE/EBOOT.PBP` on the memory stick, on custom firmware.
- **PS Vita:** install `skifree.vpk` with VitaShell (HENkaku / Ensō).
- **3DS:** `sdmc:/3ds/skifree.3dsx`, from the Homebrew Launcher.
- **Wii:** `sd:/apps/skifree/boot.dol`, from the Homebrew Channel.
- **GameCube:** boot `skifree.dol` with Swiss or another loader.
- **PS2:** run `skifree.elf` from uLaunchELF or wLaunchELF (USB or memory card).
- **PS3:** a homebrew-enabled PS3 (CFW/HEN): run `EBOOT.BIN` through a homebrew launcher, or in RPCS3.
- **PS4:** install `skifree.pkg` on a jailbroken PS4.
- **Original Xbox:** run `skifree.xbe` from a softmodded or modded Xbox, or burn `skifree.iso`.
- **Xbox 360:** `skifree.elf32` from XeLL on an RGH/JTAG console.
- **Dreamcast:** burn `skifree.cdi`, or load the ELF over a BBA or serial cable (dcload).

## How it's built

- **Shared steps:** `scripts/prep.sh` pulls the sprites and sounds out of `ski32.exe` and embeds them. `scripts/env.sh` holds the one list of game sources.
- **Per target:** `scripts/build_<target>.sh` compiles that list with the target's toolchain. Each line of `scripts/targets.tsv` names the Docker image and the command; you can run any line by hand.
- **SDL2:** most targets use their toolchain's SDL2. PS3, Dreamcast and Xbox 360 have none, so they use `src/sdl2lite/`, a small software SDL2 with a short backend per console.
