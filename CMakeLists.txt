cmake_minimum_required(VERSION 3.10)
project(ImGui_SDL2_OpenGL)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2 and OpenGL
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Find GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

# Include SDL2 and OpenGL
include_directories(${SDL2_INCLUDE_DIRS})
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
    src/Item.h
    src/Market.h
    src/MarketPopUp.h
    src/Player.h
    src/PopUp.h
    src/Events.h
    src/TextPopUp.h
    src/Button.h
    src/HospitalPopUp.h
    src/SelectPopUp.h
    src/BankPopUp.h
    src/PostPopUp.h
)

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
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${PLATFORM_LIBS} GLEW::GLEW)
