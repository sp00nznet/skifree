# SkiFree: Reborn

**The yeti is back. And this time, he compiles on everything.**

This is a static recompilation of [SkiFree](https://ski.ihoc.net/) — the legendary 1991 Windows game by Chris Pirih that traumatized an entire generation of computer users. You know the one. You're just skiing along, minding your own business, doing some sick jumps, dodging trees... and then *he* shows up.

This project takes the original `ski32.exe` binary — every function, every sprite, every pixel of that terrifying yeti — and rebuilds it from decompiled C source code to run natively on modern operating systems with a ton of new features on top.

## A Love Letter

SkiFree wasn't just a game. It was a rite of passage. It shipped with the Microsoft Entertainment Pack 3 in 1991, and for millions of kids in the '90s, it was the first video game they ever played. It lived on every school computer, every office PC, every family desktop wedged between Minesweeper and Solitaire.

And it was *perfect*. The controls were dead simple. The pixel art was charming. The physics felt right. And the abominable snowman? The abominable snowman was the first horror game boss fight most of us ever experienced, hiding in plain sight inside an innocent little ski game.

Did you know you can outrun him by pressing **F** for turbo mode? Most people didn't. Most people just... got eaten. Over and over. And kept coming back.

That's the magic of SkiFree.

## Features

### Original Game (fully preserved)
- All original sprites, physics, and game logic from ski32.exe
- Three hidden timed courses: Slalom (SS), Freestyle (FS), Giant Slalom (GS)
- The yeti. Obviously.

### New Stuff
- **Full menu bar** — File, Graphics, Sound, Controls, Multiplayer, Mods, Debug, About
- **Save/Load** — Save game state to JSON, load it back (File > Save / Load)
- **Resolution picker** — Original 4:3 (640x480 to 1600x1200) and Widescreen 16:9 (720p to 4K)
- **Sound effects** — All 9 original sound slots via SDL_mixer (multi-channel mixing)
- **Gamepad support** — Xbox controller with left stick steering, remappable buttons
- **Rebindable controls** — Keyboard and gamepad bindings configurable via ImGui dialogs
- **Replay system** — F5 to record, F6 to play back (binary format, tick-accurate)
- **AI opponents** — F7 to spawn AI skiers that race courses with obstacle avoidance
- **Competitive multiplayer** — UDP networking with lobby system, ready-up, synchronized start
- **Debug overlay** — F9 for debug info, Debug > Asset Viewer for sprite atlas
- **Mod system** — Override sprites and sounds from disk, configurable via INI
- **Enhanced physics** — Optional wind, ice patches, smooth acceleration (config toggle)

### Horde Modes (Mods menu)
- **Yeti Horde** — 20 yetis spawn in a ring around you and converge from all directions. Pure panic.
- **Tree Horde** — 20 walking trees actively chase you. When they catch you, they eat you. The forest is alive and it's hungry.

## Building

### Prerequisites

- **CMake** 3.16+
- **SDL2**, **SDL2_image**, **SDL2_ttf**, **SDL2_mixer**
- **cJSON** (via vcpkg or system package)
- A C/C++ compiler (MSVC, GCC, Clang)
- **Python 3** + `pefile` (for resource extraction)
- A copy of `ski32.exe` ([free download](https://ski.ihoc.net/))

### Step 1: Get the original resources

```bash
pip install pefile
python tools/extract_resources.py /path/to/ski32.exe
```

On Windows, also copy `C:\Windows\Fonts\vgaoem.fon` into `resources/`.

### Step 2: Get sound effects (optional)

[Alice Averlong](https://foone.wordpress.com/2017/06/20/uncovering-the-sounds-of-skifree/) tracked down the original sound files from Chris Pirih and patched them back into the binary.

```bash
# Download from the Wayback Machine:
# https://web.archive.org/web/20240324032244/http://foone.org/downloads/skifree/ski32sounds.zip
python tools/extract_resources.py /path/to/ski32sounds/ski32.exe
```

Or drop your own WAVs into `sounds/`: `ouch.wav`, `whee.wav`, `woof.wav`, `oof.wav`, `dude.wav`, `myhair.wav`, `gobble.wav`, `piddle.wav`, `argh.wav`

### Step 3: Install dependencies

**Windows (vcpkg):**
```bash
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer cjson
```

**macOS:**
```bash
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer cjson
```

**Ubuntu/Debian:**
```bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libcjson-dev
```

### Step 4: Build

```bash
cmake -B build
cmake --build build
```

Windows with vcpkg:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

## Controls

| Key | Action |
|---|---|
| **Arrow Keys** | Steer |
| **Mouse** | Point to steer |
| **Left Click** | Tricks / jump |
| **F** | Turbo mode |
| **F2** | New game |
| **F3** | Pause |
| **F5** | Toggle replay recording |
| **F6** | Play back replay |
| **F7** | Spawn AI opponent |
| **F9** | Debug overlay |
| **Enter** | Restart after crash |
| **Gamepad** | Left stick steer, A=jump, RB=turbo, Start=pause |

All keyboard and gamepad bindings are remappable via Controls menu.

## Menu Bar

| Menu | Items |
|---|---|
| **File** | Save, Save As, Load, Exit |
| **Graphics** | Resolution picker (4:3 and 16:9 options) |
| **Sound** | Sound Settings (enable + volume slider) |
| **Controls** | Keyboard config, Gamepad config (with live rebinding) |
| **Multiplayer** | Settings (username + color), Host (lobby), Join |
| **Mods** | Yeti Horde, Tree Horde |
| **Debug** | Overlay (F9), Asset Viewer |
| **About** | Credits and links |

## Multiplayer

Host a game or join one. The lobby system supports up to 4 players (humans + AI bots).

**Hosting:**
1. Set your name and color in Multiplayer > Settings
2. Multiplayer > Host > set port > Start Listening
3. Wait for players to join (they appear in the lobby list)
4. Set bot count and game modifiers (Extra Yetis, Super Speed, Star Power)
5. All humans ready up, then click Start Game
6. Everyone resets and spawns side-by-side

**Joining:**
1. Set your name and color in Multiplayer > Settings
2. Multiplayer > Join > enter IP and port > Connect
3. Click Ready in the lobby
4. Wait for host to start

**CLI:** `skifree --host [port]` or `skifree --connect <ip> [port]`

## Modding

Override any resource by creating a mod directory:

```
mods/mymod/
  resources/ski32_1.bmp    # replace any sprite
  sounds/ouch.wav          # replace any sound
  skifree.ini              # config overrides
```

Launch: `skifree --mod mods/mymod` or set `resource_dir=mods/mymod` in `skifree.ini`.

### Configuration (skifree.ini)

```ini
[game]
classic_mode=0           # 1 = original unmodified behavior

[sound]
enabled=1
volume=100               # 0-100

[physics]
enhanced=0               # wind, ice, smooth acceleration
wind_x=0
wind_y=0

[fun]
yeti_horde=0             # 20 yetis chasing you
tree_horde=0             # 20 walking trees hunting you
```

## Credits

- **Chris Pirih** — Created SkiFree in 1991. [ski.ihoc.net](https://ski.ihoc.net/)
- **Eric Fry (yuv422)** — Decompiled ski32.exe with Ghidra. [skifree_decomp](https://github.com/yuv422/skifree_decomp)
- **jeff-1amstudios** — SDL2 port of the decompiled code. [skifree_sdl](https://github.com/jeff-1amstudios/skifree_sdl)
- **Alice Averlong** — Reverse-engineered the sound system. [Blog post](https://foone.wordpress.com/2017/06/20/uncovering-the-sounds-of-skifree/)

## License

The decompiled source code is provided for educational and preservation purposes. The original game and its assets remain the property of Chris Pirih. `ski32.exe` is freely downloadable from [his website](https://ski.ihoc.net/).

---

*In loving memory of everyone who got eaten by the yeti before they knew about the F key.*

*You didn't die in vain. Probably.*
