#!/bin/bash
# Build SkiFree for every console in scripts/targets.tsv (and Android and the
# browser), from your ski32.exe, using Docker (Linux and macOS; Windows runs
# tools/setup-consoles.ps1, which does the same). Every toolchain runs in a
# container: nothing but Docker is installed on your machine.
#
#   ./setup-consoles.sh [--yes] [--game-dir <folder with ski32.exe>] [--targets wii,psp]
#
# Each step prints "[n/N] ..." and a failure ends with "Setup stopped: ...",
# which the recomp launcher shows (launcher-consoles/launcher.json). A console
# that fails to build doesn't stop the rest; the summary says which, and its
# log is in build-logs/. The builds land in builds/<console>/.
# No "set -u": macOS bash 3.2 treats an empty array as unset under it.
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
YES=0 GAME="" ONLY=""
while [ $# -gt 0 ]; do
    case "$1" in
        --yes|-y) YES=1 ;;
        --game-dir|--game) GAME="$2"; shift ;;
        --targets) ONLY=",$2,"; shift ;;
        *) echo "unknown argument: $1"; exit 2 ;;
    esac
    shift
done
mkdir -p build-logs
exec > >(tee setup.log) 2>&1

TARGETS=()
while IFS= read -r line; do
    case "$line" in \#*|"") continue ;; esac
    t="${line%%$'\t'*}"
    [ -z "$ONLY" ] || [[ "$ONLY" == *",$t,"* ]] && TARGETS+=("$line")
done < scripts/targets.tsv
N=$((2 + ${#TARGETS[@]}))

step() { echo; echo "[$1/$N] $2"; }
fail() { echo; echo "Setup stopped: $1"; echo "Details are in $ROOT/setup.log"; exit 1; }
# Containers run as root; give what they wrote back to you (Linux).
mine() { [ "$(uname)" = Linux ] && docker run --rm -v "$ROOT:/src" alpine chown -R "$(id -u):$(id -g)" "${@/#//src/}" >/dev/null 2>&1; true; }

step 1 "Checking Docker"
command -v docker >/dev/null || fail "Docker isn't installed. Install it (https://docs.docker.com/get-docker/), start it, and run this again."
docker info >/dev/null 2>&1 || fail "Docker isn't running. Start it and run this again."
echo "  Docker $(docker version --format '{{.Server.Version}}')"

step 2 "Taking the sprites and sounds out of your ski32.exe"
GAMEDIR=()
if [ -n "$GAME" ]; then
    [ -e "$GAME" ] || fail "there's nothing at '$GAME'."
    [ -f "$GAME" ] && GAME="$(dirname "$GAME")"
    GAMEDIR=(-v "$(cd "$GAME" && pwd):/game:ro")
elif [ ! -f resources/ski32_1.bmp ]; then
    [ $YES = 1 ] && fail "ski32.exe wasn't given. Pass the folder it's in with --game-dir (it's free from https://ski.ihoc.net/)."
    read -r -p "Folder with ski32.exe (free from https://ski.ihoc.net/): " GAME
    [ -d "$GAME" ] || fail "there's no folder at '$GAME'."
    GAMEDIR=(-v "$(cd "$GAME" && pwd):/game:ro")
fi
out="$(docker run --rm -u 0 -v "$ROOT:/src" "${GAMEDIR[@]}" -w /src emscripten/emsdk bash scripts/prep.sh 2>&1)"
code=$?
echo "$out" | grep -v '^Embedding' | tail -n 4 | sed 's/^/  /'
mine resources sounds build-gen
if [ $code -ne 0 ]; then
    why="$(echo "$out" | sed -n 's/.*Setup stopped: //p' | tail -n 1)"
    fail "${why:-taking the sprites out failed (the lines above say why).}"
fi

n=2; ok=(); bad=()
for line in "${TARGETS[@]}"; do
    IFS=$'\t' read -r t image cmd files state <<<"$line"
    n=$((n + 1))
    step $n "$t: building (a few minutes; the first run also downloads its toolchain)"
    # an image of our own (Android's) is built from docker/<image>.Dockerfile
    if [ -f "docker/$image.Dockerfile" ] && ! docker image inspect "$image" >/dev/null 2>&1; then
        docker build -q -t "$image" -f "docker/$image.Dockerfile" docker > "build-logs/$t.log" 2>&1 ||
            { echo "  Building the $t toolchain image failed."; bad+=("$t"); continue; }
    fi
    if docker run --rm -u 0 -v "$ROOT:/src" -w /src "$image" sh -c "$cmd" > "build-logs/$t.log" 2>&1; then
        mkdir -p "builds/$t"
        for f in $files; do
            # rm first: a copy a container left there is root's (Linux); the folder is ours
            [ -f "build-$t/$f" ] && rm -f "builds/$t/$f" && cp "build-$t/$f" "builds/$t/$f"
        done
        ok+=("$t"); echo "  Built. $state."
    else
        bad+=("$t"); echo "  Failed. The last lines of build-logs/$t.log:"; tail -n 5 "build-logs/$t.log" | sed 's/^/    /'
    fi
    mine "build-$t" builds build-logs
done

{
    echo "SkiFree console builds ($(date +%Y-%m-%d)). How to install each: docs/consoles.md."
    echo
    for line in "${TARGETS[@]}"; do
        IFS=$'\t' read -r t image cmd files state <<<"$line"
        if [[ " ${ok[*]} " == *" $t "* ]]; then echo "  $t: built. $state."; else echo "  $t: FAILED (build-logs/$t.log)"; fi
    done
} > builds/README.txt 2>/dev/null || true

echo; cat builds/README.txt 2>/dev/null
[ ${#ok[@]} -gt 0 ] || fail "no console built. The logs in build-logs/ say why."

launcher="Console builds (recomp).sh"
[ "$(uname)" = Darwin ] && launcher="Console builds (recomp).command"
printf '#!/bin/sh\n# Opens the folder with the console builds.\ncd "$(dirname "$0")/builds" && { open . 2>/dev/null || xdg-open .; }\n' > "$launcher"
chmod +x "$launcher"
echo
echo "Done: ${#ok[@]} of ${#TARGETS[@]} consoles built${bad:+ (failed: ${bad[*]})}. They're in builds/."
