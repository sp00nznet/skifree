#!/bin/bash
# Linux and macOS setup: the same steps as Setup.cmd (tools/setup.ps1).
#
#   1. check the tools (a C/C++ compiler, CMake, Python 3, SDL2 and
#      SDL2_ttf); on a Mac it installs what's missing with Homebrew, on
#      Linux it says which packages to install
#   2. pull the sprites (and sounds, from ski32sounds.exe) out of your ski32.exe
#   3. build build-linux/skifree (or build-macos/skifree)
#   4. leave a launcher here: SkiFree (recomp).sh, or .command on a Mac
#
#   ./setup.sh                        # looks here and in ~/Downloads for ski32*.exe / ski32*.zip
#   ./setup.sh ~/Downloads/ski32.exe  # the file, or the folder it is in
#   ./setup.sh --yes --game-dir DIR   # as the recomp launcher runs it
#   ./setup.sh --targets pc,psp,wii   # pc is this computer (the default); the rest are
#                                     # scripts/targets.tsv's, built by setup-consoles.sh in Docker
#
# ski32.exe is free from its author: https://ski.ihoc.net/
set -e
cd "$(dirname "$0")"
ROOT=$PWD
GAME_ARG=; YES=; TARGETS=pc
while [ $# -gt 0 ]; do
  case $1 in
    --yes) YES=--yes ;;                        # nothing here asks; passed on to setup-consoles.sh
    --game-dir) GAME_ARG=$2; shift ;;
    --targets) TARGETS=${2:-pc}; shift ;;
    *) GAME_ARG=$1 ;;
  esac
  shift
done

say()  { printf '%s\n' "$*"; }
step() { printf '\n\033[36m%s\033[0m\n' "$*"; }
fail() { printf '\n\033[31mSetup stopped: %s\033[0m\n' "$*" >&2; exit 1; }
MAC=0; [ "$(uname)" = Darwin ] && MAC=1
# Every other target is setup-consoles.sh's, which downloads only their toolchains.
OTHERS=$(echo ",$TARGETS," | sed 's/,pc,/,/g; s/^,*//; s/,*$//')
consoles() {
  args=(--targets "$OTHERS"); [ -n "$YES" ] && args+=("$YES"); [ -n "$GAME_ARG" ] && args+=(--game-dir "$GAME_ARG")
  exec bash "$ROOT/setup-consoles.sh" "${args[@]}"
}
case ",$TARGETS," in *,pc,*) ;; *) [ -z "$OTHERS" ] || consoles ;; esac

# ---- 1. tools -------------------------------------------------------------------
step "[1/4] Checking the tools"
if [ $MAC = 1 ]; then
  xcode-select -p >/dev/null 2>&1 || { xcode-select --install 2>/dev/null; fail "Xcode's command line tools are needed: accept the install that just opened, then run this again."; }
  command -v brew >/dev/null || fail "Homebrew is needed: https://brew.sh"
  need=()
  for f in cmake pkgconf sdl2 sdl2_ttf; do brew list --formula "$f" >/dev/null 2>&1 || need+=("$f"); done
  if [ ${#need[@]} -gt 0 ]; then
    say "  Installing ${need[*]} with Homebrew"
    brew install "${need[@]}" || fail "brew install ${need[*]} failed."
  fi
else
  need=()
  for t in cc c++ cmake python3 pkg-config; do command -v "$t" >/dev/null || need+=("$t"); done
  if command -v pkg-config >/dev/null; then
    for p in sdl2 SDL2_ttf; do pkg-config --exists $p || need+=("$p's development files"); done
  fi
  if [ ${#need[@]} -gt 0 ]; then
    say "  Missing: ${need[*]}. From your package manager, for example:"
    say "    Debian, Ubuntu: sudo apt install build-essential cmake python3 pkg-config libsdl2-dev libsdl2-ttf-dev"
    say "    Fedora:         sudo dnf install gcc gcc-c++ cmake python3 pkgconf SDL2-devel SDL2_ttf-devel"
    say "    Arch:           sudo pacman -S base-devel cmake python pkgconf sdl2 sdl2_ttf"
    fail "install them, then run ./setup.sh again."
  fi
fi
command -v python3 >/dev/null || fail "Python 3 is needed."
say "  ok"

# ---- 2. the game ----------------------------------------------------------------
# ski32sounds.exe (Alice Averlong's, with the sounds) over ski32.exe; a zip of either.
pick() { ls "$@" 2>/dev/null | grep -i sounds | head -1 || true; ls "$@" 2>/dev/null | head -1; }
step "[2/4] Taking the sprites and sounds out of your ski32.exe"
if [ -n "$GAME_ARG" ] && [ -f "$GAME_ARG" ]; then src=$GAME_ARG
else
  dirs=("${GAME_ARG:-$ROOT}"); [ -z "$GAME_ARG" ] && dirs+=("$HOME/Downloads")
  src=
  for d in "${dirs[@]}"; do
    src=$(pick "$d"/[Ss][Kk][Ii]32*.[Ee][Xx][Ee] "$d"/[Ss][Kk][Ii]32*.[Zz][Ii][Pp] | head -1)
    [ -n "$src" ] && break
  done
fi
if [ -z "$src" ]; then
  [ -f resources/ski32_1.bmp ] && { say "  none found, but resources/ already has the sprites: keeping them"; }
  [ -f resources/ski32_1.bmp ] || fail "ski32.exe was not found. Get it free from https://ski.ihoc.net/ and run ./setup.sh /path/to/ski32.exe"
else
  say "  $src"
  case $src in
    *.[Zz][Ii][Pp])
      tmp=$(mktemp -d)
      python3 -m zipfile -e "$src" "$tmp" || fail "could not unzip $src"
      exe=$(pick "$tmp"/*.[Ee][Xx][Ee] | head -1)
      [ -n "$exe" ] || fail "$src has no .exe in it."
      python3 tools/extract_resources.py "$exe" || fail "could not read $exe: is it SkiFree's ski32.exe?"
      rm -rf "$tmp" ;;
    *) python3 tools/extract_resources.py "$src" || fail "could not read $src: is it SkiFree's ski32.exe?" ;;
  esac
fi

# ---- 3. build -------------------------------------------------------------------
B=build-linux; [ $MAC = 1 ] && B=build-macos
step "[3/4] Building $B/skifree (a minute or two)"
jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
cmake -S . -B "$B" -DCMAKE_BUILD_TYPE=Release >/dev/null || fail "CMake could not configure the build."
cmake --build "$B" -j "$jobs" || fail "the build failed."
[ -x "$B/skifree" ] || fail "the build finished but $B/skifree is missing."

# ---- 4. launcher ----------------------------------------------------------------
L="SkiFree (recomp).sh"; [ $MAC = 1 ] && L="SkiFree (recomp).command"
printf '#!/bin/sh\ncd "$(dirname "$0")" && exec %s/skifree "$@"\n' "$B" > "$L"
chmod +x "$L"
step "[4/4] Ready: ./$L"
say "Move the mouse to the top of the window for the menus. F to go faster; the yeti is waiting."
[ -z "$OTHERS" ] || { echo; consoles; }
