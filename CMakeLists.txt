cmake_minimum_required(VERSION 3.16)
project(Beijing_Fushengji VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(FSJ_BUILD_APP "Build the SDL2/OpenGL game" ON)
option(FSJ_BUILD_TESTS "Build the unit tests" ON)
option(FSJ_ENABLE_LLM "Build the libcurl based LLM client (offline events always work)" ON)
option(FSJ_FETCH_DEPENDENCIES "Download SDL2 / nlohmann_json / GoogleTest when no system copy is found" ON)
option(FSJ_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

include(FetchContent)

# --- Compiler settings -----------------------------------------------------------
if(MSVC)
  add_compile_options(/utf-8)  # the sources contain UTF-8 Chinese string literals
  add_compile_definitions(NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

function(fsj_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(FSJ_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(FSJ_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

# --- nlohmann_json (needed by the core: LLM request/response handling) ----------
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

# --- Game core: no window, no network, fully unit-testable -------------------------
add_library(fsj_core STATIC
  src/core/AiConfig.cpp
  src/core/Events.cpp
  src/core/Game.cpp
  src/core/Item.cpp
  src/core/Language.cpp
  src/core/LlmEventSource.cpp
  src/core/Market.cpp
  src/core/Player.cpp
  src/core/RandomEvent.cpp)
target_include_directories(fsj_core PUBLIC src/core)
target_link_libraries(fsj_core PUBLIC nlohmann_json::nlohmann_json)
fsj_enable_warnings(fsj_core)

# --- LLM transport (optional libcurl) and event-source factory --------------------
add_library(fsj_ai STATIC src/ai/EventSourceFactory.cpp)
target_include_directories(fsj_ai PUBLIC src/ai)
target_link_libraries(fsj_ai PUBLIC fsj_core)
fsj_enable_warnings(fsj_ai)
set(FSJ_HAVE_CURL OFF)
if(FSJ_ENABLE_LLM)
  find_package(CURL)
  if(CURL_FOUND)
    set(FSJ_HAVE_CURL ON)
    target_sources(fsj_ai PRIVATE src/ai/CurlChatClient.cpp)
    target_link_libraries(fsj_ai PUBLIC CURL::libcurl)
    target_compile_definitions(fsj_ai PUBLIC FSJ_HAVE_CURL)
  else()
    message(WARNING "libcurl not found: building without the LLM client, only offline events will be available")
  endif()
endif()

# --- The game ---------------------------------------------------------------------------
if(FSJ_BUILD_APP)
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
  find_package(OpenGL REQUIRED)

  # Dear ImGui is vendored (trimmed to what the game compiles).
  add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_sdl2.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp)
  target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
  target_link_libraries(imgui PUBLIC SDL2::SDL2 OpenGL::GL ${CMAKE_DL_LIBS})

  add_executable(Beijing_Fushengji
    src/ui/main.cpp
    src/ui/BankPopUp.cpp
    src/ui/HospitalPopUp.cpp
    src/ui/MarketPopUp.cpp
    src/ui/PopUp.cpp
    src/ui/PostPopUp.cpp
    src/ui/SelectPopUp.cpp
    src/ui/TextPopUp.cpp)
  target_include_directories(Beijing_Fushengji PRIVATE src/ui)
  target_link_libraries(Beijing_Fushengji PRIVATE fsj_ai fsj_core imgui SDL2::SDL2)
  if(TARGET SDL2::SDL2main)
    target_link_libraries(Beijing_Fushengji PRIVATE SDL2::SDL2main)
  endif()
  fsj_enable_warnings(Beijing_Fushengji)

  # The CJK font is looked up next to the executable at run time.
  add_custom_command(TARGET Beijing_Fushengji POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Beijing_Fushengji>/Font
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/Font/wqy-zenhei.ttc
            $<TARGET_FILE_DIR:Beijing_Fushengji>/Font/wqy-zenhei.ttc)

  include(GNUInstallDirs)
  install(TARGETS Beijing_Fushengji RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(DIRECTORY Font/ DESTINATION share/beijing-fushengji/Font)
endif()

# --- Tests ------------------------------------------------------------------------------
if(FSJ_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
