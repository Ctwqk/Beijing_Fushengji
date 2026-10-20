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

## Independent review (2026-10-19)

A fresh tracked-file snapshot of `6c55ab3` was built using the documented Dockerfile and quickstart commands.
The existing 81 CTest entries passed, as did the real-window Chinese/English GUI checks. The Chinese screenshot
was visually inspected. The branch's existing CI run `39345028320` also passed all three jobs; its Windows log
confirmed a real MSVC executable build plus CLI and local HTTP test execution.

New regressions reproduced failures that the existing suite missed:

* Malformed JSON inside an HTTP 200 model reply exposed fake credentials and endpoint text through parser diagnostics.
* A transport failure exposed private URL details through libcurl's detailed error buffer.
* Escaped NULs and ImGui label syntax were accepted in event text. An embedded NUL in the title produced an invisible
  event popup that left travel locked; the real-window test timed out waiting for a visible choice.

The rule comparison used `origin/main`: opening balances, catalogue prices, 5% debt interest, 4% savings interest,
1750 yuan per health point, 100 slots and reputation changes per sale were retained. The README now distinguishes
these constants from the changed price process and event frequencies. Hospital tests use literal expected cash
amounts instead of deriving both input and expectation from the production price constant; catalogue tests pin
all seven original prices. Test comments describe the offline table's actual event-level downside guarantee.

The declared CMake minimum now matches the documented `ctest --test-dir` command, which was
[added in CMake 3.20](https://cmake.org/cmake/help/v3.20/release/3.20.html#ctest).

After the parser/transport fixes, all 83 CTest entries passed (81 core GoogleTest cases, 13 HTTP cases grouped in
one entry, and CLI validation). Deliberately changing the hospital cost to 1749 and the first catalogue price to
207 in the disposable snapshot caused all three targeted rule tests to fail. The original files were restored
and rebuilt before further checks.

The expanded GUI test confirmed that an invalid event title now falls back to a visible, usable event and travel
resumes. A separate valid 120-byte choice (40 Chinese characters) then reproduced clipped text in the fixed-height
buttons. The rendering fix wraps the complete accepted label and sizes the button to fit; its regression checks
both button height and actual dark glyph pixels beyond a single 18 px text line.

The final protected run passed all 83 CTest entries and the expanded GUI regression. The long-choice screenshot
was visually inspected: all three choices display their complete 40-character labels on two lines. Both the
source-tree executable and the installed executable launched the Chinese offline UI under Xvfb; the installed
copy ran from `/tmp`, and both reached the expected 10-second timeout (exit 124).

Two additional configurations were built with warnings treated as errors and dependency fetching disabled:

```bash
cmake -S . -B build-offline -G Ninja -DFSJ_ENABLE_LLM=OFF \
  -DFSJ_FETCH_DEPENDENCIES=OFF -DFSJ_WARNINGS_AS_ERRORS=ON
cmake --build build-offline -j4
ctest --test-dir build-offline --output-on-failure

cmake -S . -B build-headless -G Ninja -DFSJ_BUILD_APP=OFF -DFSJ_ENABLE_LLM=OFF \
  -DFSJ_FETCH_DEPENDENCIES=OFF -DFSJ_WARNINGS_AS_ERRORS=ON
cmake --build build-headless -j4
ctest --test-dir build-headless --output-on-failure
```

The offline app passed 82/82 CTest entries; the headless core passed 81/81. All local builds, HTTP integration tests
and GUI runs used the global lock and memory limits described above. No live-provider calls were needed.
