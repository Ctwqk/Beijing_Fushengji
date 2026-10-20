# Verification record

Verified on 2026-10-19 from a clean tracked-file snapshot using the Ubuntu 24.04 image in
`docker/Dockerfile`, GCC 13.3 and Mesa software OpenGL. No previous build tree was copied.

Local heavy work was serialized with the owner's global lock and a systemd scope capped at 6 GiB with no swap.
Each build/demo container was separately limited to 4 GiB, no swap, and four CPUs; builds used at most four jobs.
Available host memory was checked against the required 8 GiB threshold before starting.

## Commands and results

The following commands ran inside the Docker image, from the clean source tree:

```bash
cmake -S . -B build -G Ninja -DFSJ_WARNINGS_AS_ERRORS=ON
cmake --build build -j4
ctest --test-dir build --output-on-failure
python3 tests/test_gui.py build/Beijing_Fushengji build/gui-test

cmake -S . -B build-headless -G Ninja -DFSJ_BUILD_APP=OFF -DFSJ_ENABLE_LLM=OFF \
  -DFSJ_FETCH_DEPENDENCIES=OFF -DFSJ_WARNINGS_AS_ERRORS=ON
cmake --build build-headless -j4
ctest --test-dir build-headless --output-on-failure
```

| Check | Result |
|---|---|
| Full app, warnings treated as errors | Build passed |
| Full CTest suite | 81/81 entries passed: 79 core GoogleTest cases, 11 HTTP cases in one entry, and CLI validation |
| Real-window GUI regression | Chinese screenshot and clean exit; delayed local AI response blocks travel; travel resumes after the choice |
| Headless build without libcurl | Build passed; 79/79 tests passed |

Installation was checked with `cmake --install build --prefix /src/build/install`. From `/tmp`, outside the source
directory, the following launched the installed Chinese UI and reached the expected timeout exit code 124:

```bash
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a timeout --kill-after=3s 10 \
  /src/build/install/bin/Beijing_Fushengji --lang zh --offline
```

## Regression evidence

Before the fixes, the new tests reproduced credential echoes in both HTTP 401 and HTTP 200 error envelopes,
incorrect signs after converting enormous JSON numbers, acceptance of malformed/out-of-range CLI values, and
advancing from day 1 to day 2 while an AI event was pending. The same tests passed after the fixes.

The GUI test uses the real SDL/OpenGL/ImGui application and a deliberately delayed local HTTP server. Its images
are uploaded by Linux CI. Chinese glyphs were also visually inspected in the locally captured screenshot.

## Demo assets

`IMAGE=pu-beijing-takeover-dev DOCKER_NAME=pu-beijing-demo-verify scripts/capture_demo.sh` completed in the clean
snapshot, using the same Dockerfile-built toolchain. It replayed the language selector and seeded Chinese/English
tours under Xvfb, including trading, town services, random events and the ending. The resulting eight PNGs in
`docs/screenshots/` and `docs/demo.gif` were copied back into the repository. Each PNG is below 81 KB; the GIF is
about 615 KB. The Chinese event and English ending captures were visually inspected.

## Scope

No paid or live-provider API calls were used. HTTP behavior is exercised against a local mock server. Windows is
built and tested by GitHub Actions; interactive Windows desktop behavior and macOS remain unverified. See the
README's CI badge for the current Linux and Windows workflow status.
