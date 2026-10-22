#!/bin/bash
# The step every console shares: take the sprites and sounds out of the
# player's ski32.exe (mounted at /game, or $1) and embed them in
# build-gen/embedded_resources.h. Needs python3 and cmake (the emscripten
# image has both). Skips the extraction when resources/ already has them.
set -e
cd "$(dirname "$0")/.."
GAME="${1:-/game}"
if [ ! -f resources/ski32_1.bmp ] || [ -d "$GAME" ]; then
    src=$(ls "$GAME"/[Ss][Kk][Ii]32*sounds*.[EeZz][XxIi][EePp] 2>/dev/null | head -1)
    [ -n "$src" ] || src=$(ls "$GAME"/[Ss][Kk][Ii]32*.[EeZz][XxIi][EePp] 2>/dev/null | head -1)
    if [ -n "$src" ]; then
        case "$src" in
            *.[Zz][Ii][Pp])
                tmp=$(mktemp -d)
                python3 -m zipfile -e "$src" "$tmp"
                exe=$(ls "$tmp"/*.[Ee][Xx][Ee] | head -1)
                python3 tools/extract_resources.py "$exe"
                rm -rf "$tmp" ;;
            *) python3 tools/extract_resources.py "$src" ;;
        esac
    fi
fi
[ -f resources/ski32_1.bmp ] || { echo "Setup stopped: ski32.exe was not found. Get it free from https://ski.ihoc.net/"; exit 1; }
mkdir -p build-gen
cd build-gen
cmake "-DSOURCE_DIR=$PWD/.." -P ../cmake/EmbedResources.cmake >/dev/null
echo "Embedded $(grep -c '\.name = ' embedded_resources.h) files in build-gen/embedded_resources.h"
