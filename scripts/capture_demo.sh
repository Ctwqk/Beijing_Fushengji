#!/usr/bin/env bash
# Rebuilds the game in Docker and regenerates docs/screenshots/*.png and docs/demo.gif by
# playing the real game (offline, fixed seeds) under Xvfb with Mesa software rendering.
#
#   scripts/capture_demo.sh
#
# Needs Docker with network access to build docker/Dockerfile. Nothing is installed on the host.
set -euo pipefail
cd "$(dirname "$0")/.."

IMAGE="${IMAGE:-pu-beijing-capture}"
# The classic builder applies these limits to the dependency-install container too.
DOCKER_BUILDKIT=0 docker build --memory=4g --memory-swap=4g -q -t "$IMAGE" -f docker/Dockerfile docker >/dev/null

docker run --rm --name "${DOCKER_NAME:-pu-beijing-capture}" --memory=4g --memory-swap=4g --cpus=4 \
  --user "$(id -u):$(id -g)" -e HOME=/tmp \
  -v "$PWD":/src -w /src "$IMAGE" bash -lc '
set -euo pipefail
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build build -j4
EXE=build/Beijing_Fushengji
OUT=build/demo
rm -rf "$OUT"; mkdir -p "$OUT" docs/screenshots

capture() { python3 scripts/capture_demo.py --exe "$EXE" "$@" >/dev/null; }
capture --out "$OUT/language"  --scenario language
capture --out "$OUT/zh"        --scenario tour-zh --seed 11 --days 7
capture --out "$OUT/en"        --scenario tour-en --seed 5  --days 6 --game-days 6
capture --out "$OUT/gif"       --scenario tour-zh --seed 5  --days 6 --game-days 6

S=docs/screenshots
cp "$OUT/language/language.png"  $S/01-language-select.png
cp "$OUT/zh/main-day3.png"       $S/02-main-zh.png
cp "$OUT/zh/trade.png"           $S/03-trade-zh.png
cp "$OUT/zh/popup-day1-2.png"    $S/04-random-event-zh.png
cp "$OUT/zh/popup-day1-1.png"    $S/05-news-zh.png
cp "$OUT/en/main-day3.png"       $S/06-main-en.png
cp "$OUT/en/popup-day1-1.png"    $S/07-random-event-en.png
cp "$OUT/en/popup-day6-2.png"    $S/08-ending-en.png

# GIF: one frame every 0.7 s, palette optimised to stay small.
ffmpeg -loglevel error -y -framerate 1.5 -i "$OUT/gif/frame%03d.png" \
  -vf "scale=600:-1:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=48:stats_mode=diff[p];[s1][p]paletteuse=dither=none" \
  docs/demo.gif
ls -la docs/demo.gif docs/screenshots
'
