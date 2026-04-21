# Beijing_Fushengji

A C++ life-simulation / narrative trading game built with SDL2, OpenGL, and ImGui.

The project mixes market simulation, popup-driven game interactions, bilingual UI support, and LLM-backed random events into a desktop gameplay prototype themed around surviving and making money in Beijing.

## Highlights

- Desktop UI built with SDL2 + OpenGL + ImGui
- Multiple gameplay popup systems for market, hospital, bank, post, and text interactions
- Bilingual text support with Chinese font loading
- Event-driven life simulation loop with inventory, debt, health, and reputation state
- AI-backed random event generation through an external chat-completions API wrapper

## Tech Stack

- C++17
- SDL2
- OpenGL
- GLEW
- ImGui
- libcurl
- nlohmann/json
- CMake

## Repository Layout

```text
src/
├── main.cpp          # Main game loop and window setup
├── Market*.cpp       # Market interactions
├── *PopUp.cpp        # UI popups and interaction windows
├── Events.cpp        # Event flow
└── AiApi.cpp         # LLM-backed event integration

include/              # Headers for gameplay systems
utils/                # Language utilities and helpers
Font/                 # Font assets for multilingual rendering
imgui/                # Dear ImGui sources
```

## Building

### Prerequisites

- CMake 3.10+
- SDL2
- OpenGL
- GLEW
- libcurl

### Build

```bash
cmake -S . -B build
cmake --build build
```

The generated executable is the `Beijing_Fushengji` target defined in `CMakeLists.txt`.

## Notes

- The codebase includes bundled ImGui sources and expects local system libraries for SDL2 / OpenGL / curl.
- Some gameplay content is specific to the project theme and Chinese-language UI assets.
