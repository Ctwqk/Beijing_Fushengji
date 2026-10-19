cmake_minimum_required(VERSION 3.16)
project(Beijing_Fushengji VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(FSJ_FETCH_DEPENDENCIES "Download SDL2 / nlohmann_json when no system copy is found" ON)

include(FetchContent)

# --- SDL2 ------------------------------------------------------------------
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
  if(NOT FSJ_FETCH_DEPENDENCIES)
    message(FATAL_ERROR "SDL2 not found (install libsdl2-dev or enable FSJ_FETCH_DEPENDENCIES)")
  endif()
  message(STATUS "SDL2 not found on the system, fetching it")
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-2.30.9.tar.gz)
  FetchContent_MakeAvailable(SDL2)
endif()

# --- OpenGL, libcurl, nlohmann_json -----------------------------------------
find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.9 QUIET)
if(NOT nlohmann_json_FOUND)
  if(NOT FSJ_FETCH_DEPENDENCIES)
    message(FATAL_ERROR "nlohmann_json not found (install nlohmann-json3-dev or enable FSJ_FETCH_DEPENDENCIES)")
  endif()
  message(STATUS "nlohmann_json not found on the system, fetching it")
  FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# --- Dear ImGui (vendored, trimmed to what the game uses) ---------------------
add_library(imgui STATIC
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/imgui_widgets.cpp
  third_party/imgui/backends/imgui_impl_sdl2.cpp
  third_party/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
target_link_libraries(imgui PUBLIC SDL2::SDL2 OpenGL::GL ${CMAKE_DL_LIBS})

# --- Game -------------------------------------------------------------------
add_executable(Beijing_Fushengji
  src/ui/main.cpp
  src/ui/PopUp.cpp
  src/ui/BankPopUp.cpp
  src/ui/HospitalPopUp.cpp
  src/ui/MarketPopUp.cpp
  src/ui/PostPopUp.cpp
  src/ui/SelectPopUp.cpp
  src/ui/TextPopUp.cpp
  src/core/Events.cpp
  src/core/Language.cpp
  src/core/Market.cpp
  src/core/Player.cpp
  src/ai/AiApi.cpp)
target_include_directories(Beijing_Fushengji PRIVATE src/core src/ui src/ai)
target_link_libraries(Beijing_Fushengji PRIVATE imgui SDL2::SDL2 CURL::libcurl nlohmann_json::nlohmann_json)
if(TARGET SDL2::SDL2main)
  target_link_libraries(Beijing_Fushengji PRIVATE SDL2::SDL2main)
endif()

# The CJK font is looked up next to the executable at run time.
add_custom_command(TARGET Beijing_Fushengji POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Beijing_Fushengji>/Font
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/Font/wqy-zenhei.ttc
          $<TARGET_FILE_DIR:Beijing_Fushengji>/Font/wqy-zenhei.ttc)
