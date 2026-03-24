# SkiFree: Reborn

```
        \O/        ___
         |        /   \
        / \      | SKI |
       /   \      \   /
      /     \      ---
     /  ___  \
    /  / F \  \
   /  /     \  \
  /__/       \__\
```

**The yeti is back. And this time, he compiles on everything.**

This is a static recompilation of [SkiFree](https://ski.ihoc.net/) — the legendary 1991 Windows game by Chris Pirih that traumatized an entire generation of computer users. You know the one. You're just skiing along, minding your own business, doing some sick jumps, dodging trees... and then *he* shows up.

This project takes the original `ski32.exe` binary — every function, every sprite, every pixel of that terrifying yeti — and rebuilds it from decompiled C source code to run natively on modern operating systems. No emulators. No compatibility layers. Just pure, unadulterated SkiFree, the way Chris intended it (well, close enough).

## A Love Letter

SkiFree wasn't just a game. It was a rite of passage. It shipped with the Microsoft Entertainment Pack 3 in 1991, and for millions of kids in the '90s, it was the first video game they ever played. It lived on every school computer, every office PC, every family desktop wedged between Minesweeper and Solitaire.

And it was *perfect*. The controls were dead simple. The pixel art was charming. The physics felt right. And the abominable snowman? The abominable snowman was the first horror game boss fight most of us ever experienced, hiding in plain sight inside an innocent little ski game.

Did you know you can outrun him by pressing **F** for turbo mode? Most people didn't. Most people just... got eaten. Over and over. And kept coming back.

That's the magic of SkiFree.

## How It Works

In 2022, [Eric Fry (yuv422)](https://github.com/yuv422/skifree_decomp) used Ghidra to decompile `ski32.exe` back into C source code — all 108 functions, every data table, every sprite offset. [jeff-1amstudios](https://github.com/jeff-1amstudios/skifree_sdl) then ported that decompiled code to SDL2, replacing the Win32 GDI rendering calls with cross-platform equivalents.

This project builds on both of their incredible work: cleaning up the source, fixing bugs, restoring mouse control, and packaging it all up so you can build and play it on Windows, macOS, and Linux with modern compilers and toolchains.

The result is a playable binary that is faithful to the original game logic — same physics, same sprites, same yeti. Just freed from its Win16/Win32 cage.

## Building

### Prerequisites

- **CMake** 3.16+
- **SDL2**, **SDL2_image**, **SDL2_ttf**
- A C compiler (GCC, Clang, MSVC, etc.)
- **Python 3** + `pefile` (for resource extraction only)
- A copy of `ski32.exe` (free download from [ski.ihoc.net](https://ski.ihoc.net/))

### Step 1: Get the original resources

SkiFree's sprite bitmaps are the original artwork by Chris Pirih. Out of respect for his work, they aren't redistributed here — you need to extract them from the freely available `ski32.exe`:

```bash
# Install the extraction dependency
pip install pefile

# Download ski32.exe from https://ski.ihoc.net/ and extract resources
python tools/extract_resources.py /path/to/ski32.exe
```

You'll also need a VGA OEM font file for the status display. On Windows, copy `C:\Windows\Fonts\vgaoem.fon` into the `resources/` directory. On other platforms, any small monospace `.ttf` will work.

### Step 1.5: Get sound effects (optional)

The original SkiFree had code for 9 sound effects but shipped silent — no WAV files were ever embedded. In 2017, [Foone Turing](https://foone.wordpress.com/2017/06/20/uncovering-the-sounds-of-skifree/) tracked down the original sound files from Chris Pirih himself and patched them back in.

```bash
# Download Foone's sound-enabled version from the Wayback Machine
# https://web.archive.org/web/20240324032244/http://foone.org/downloads/skifree/ski32sounds.zip

# Extract the WAVs from the modified exe
python tools/extract_resources.py /path/to/ski32sounds/ski32.exe
```

Or just drop your own WAV files into a `sounds/` directory:
`ouch.wav`, `whee.wav`, `woof.wav`, `oof.wav`, `dude.wav`, `myhair.wav`, `gobble.wav`, `piddle.wav`, `argh.wav`

### Step 2: Install SDL2

**Windows (vcpkg):**
```bash
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer
```

**macOS:**
```bash
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

**Ubuntu/Debian:**
```bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

**Fedora:**
```bash
sudo dnf install SDL2-devel SDL2_image-devel SDL2_ttf-devel SDL2_mixer-devel
```

**Arch:**
```bash
sudo pacman -S sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

### Step 3: Build

```bash
cmake -B build
cmake --build build
```

On Windows with vcpkg:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

The executable will be at `build/skifree` (or `build/Release/skifree.exe` on Windows).

## Controls

| Key | Action |
|---|---|
| **Arrow Keys** | Steer your skier |
| **Mouse** | Point to steer (yes, it always supported this!) |
| **Left Click** | Tricks / jump |
| **F** | **TURBO MODE** (the yeti's worst nightmare) |
| **F2** | New game |
| **F3** | Pause / unpause |
| **Enter** | Restart after crashing |
| **Esc** | Minimize |

### Game Modes

There are three timed courses hidden in plain sight:

- **Slalom (SS)** — Ski through the left-side gates (flag poles)
- **Freestyle (FS)** — Ski the center run, score style points with tricks
- **Giant Slalom (GS)** — Ski through the right-side gates

Most people never realized these existed. They just skied straight down into the yeti's waiting arms.

### Pro Tips

- The **F key** enables turbo mode. This is how you outrun the yeti.
- Jump over dogs to earn style points. Jump over burning trees for even more.
- Hit a stump while going uphill to reveal a mushroom (hidden Easter egg from the 32-bit version!)
- Trees can catch fire if you crash into them while airborne.
- Walking trees exist. Yes, really. They're rare. Watch the edges of the screen.

## Modding

SkiFree is fully moddable. Create a mod directory and override any resource:

```
mods/mymod/
  resources/ski32_1.bmp    # replace any sprite
  sounds/ouch.wav          # replace any sound effect
  skifree.ini              # override game settings
```

Launch with your mod:
```bash
skifree --mod mods/mymod
```

Or set it in `skifree.ini`:
```ini
[mods]
resource_dir=mods/mymod
```

The resource system checks your mod directory first, then falls back to the embedded originals. You can replace individual sprites without touching the rest.

### Configuration

Copy `skifree.ini.example` to `skifree.ini` to customize:

```ini
[game]
classic_mode=0        # 1 = original unmodified behavior

[sound]
enabled=1
volume=128

[physics]
enhanced=0            # 1 = wind, ice, smooth acceleration
```

## Credits

- **Chris Pirih** — Created SkiFree in 1991. A true legend. [ski.ihoc.net](https://ski.ihoc.net/)
- **Eric Fry (yuv422)** — Decompiled ski32.exe back to C using Ghidra. Herculean effort. [skifree_decomp](https://github.com/yuv422/skifree_decomp)
- **jeff-1amstudios** — Ported the decompiled code to SDL2. [skifree_sdl](https://github.com/jeff-1amstudios/skifree_sdl)
- **Foone Turing** — Reverse-engineered the sound system and mapped all the sound effects. [Blog post](https://foone.wordpress.com/2017/06/20/uncovering-the-sounds-of-skifree/)

## License

The decompiled source code is provided for educational and preservation purposes. The original game and its assets remain the property of Chris Pirih. `ski32.exe` is freely downloadable from [his website](https://ski.ihoc.net/).

---

*In loving memory of everyone who got eaten by the yeti before they knew about the F key.*

*You didn't die in vain. Probably.*
