cmake_minimum_required(VERSION 3.10)
project(Beijing_Fushengji)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

# Fetch SDL2
set(SDL2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/SDL2)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.26.5
    SOURCE_DIR ${SDL2_SOURCE_DIR}
)
FetchContent_MakeAvailable(SDL2)

#Fetch GLEW
set(GLEW_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/GLEW)
FetchContent_Declare(
    GLEW
    GIT_REPOSITORY https://github.com/nigels-com/glew.git
    GIT_TAG glew-2.2.0
    SOURCE_DIR ${GLEW_SOURCE_DIR}
)
FetchContent_MakeAvailable(GLEW)
find_package(GLEW 2.0 REQUIRED)
#Find OpenGL
find_package(OpenGL REQUIRED)

#ImGui
include_directories(${CMAKE_SOURCE_DIR}/imgui)
include_directories(${CMAKE_SOURCE_DIR}/imgui/backends)


# Source Files
set(SOURCES
    src/main.cpp
    src/Market.cpp
    src/Player.cpp
    src/MarketPopUp.cpp
    src/TextPopUp.cpp
    src/HospitalPopUp.cpp
    src/PopUp.cpp
    src/BankPopUp.cpp
    src/SelectPopUp.cpp
    src/PostPopUp.cpp
    src/Events.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
set(HEADER
    include/Item.h
    include/Market.h
    include/MarketPopUp.h
    include/Player.h
    include/PopUp.h
    include/Events.h
    include/TextPopUp.h
    include/Button.h
    include/HospitalPopUp.h
    include/SelectPopUp.h
    include/BankPopUp.h
    include/PostPopUp.h
)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Platform-Specific Dependencies
if(WIN32)
    set(PLATFORM_LIBS opengl32)
elseif(APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    set(PLATFORM_LIBS ${OpenGL_LIBRARY})
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    set(PLATFORM_LIBS ${OPENGL_gl_LIBRARY})
endif()

# Add Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADER})

# Link Libraries
target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main ${PLATFORM_LIBS} GLEW::GLEW)
