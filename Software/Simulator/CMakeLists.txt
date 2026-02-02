cmake_minimum_required(VERSION 3.16)
project(MegaCubeSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Fetch GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Path to LED Display source (for shared code)
set(LED_DISPLAY_SRC "${CMAKE_SOURCE_DIR}/../LED Display/src")

# Simulator sources
set(SIMULATOR_SOURCES
    src/main.cpp
    src/Renderer.cpp
    src/SimDisplay.cpp
)

# Shared sources from LED Display (math, noise, color, animations)
set(SHARED_SOURCES
    ${LED_DISPLAY_SRC}/power/Math3D.cpp
    ${LED_DISPLAY_SRC}/power/Noise.cpp
    ${LED_DISPLAY_SRC}/power/Color.cpp
    ${LED_DISPLAY_SRC}/power/Particle.cpp
    ${LED_DISPLAY_SRC}/power/Timer.cpp
)

add_executable(simulator ${SIMULATOR_SOURCES} ${SHARED_SOURCES})

# Include our Arduino shim FIRST so it overrides the real Arduino.h
target_include_directories(simulator PRIVATE
    src                        # Our shims (Arduino.h) - MUST be first
    ${LED_DISPLAY_SRC}
    ${LED_DISPLAY_SRC}/core
    ${LED_DISPLAY_SRC}/power
    ${LED_DISPLAY_SRC}/space
)

target_link_libraries(simulator PRIVATE
    OpenGL::GL
    glfw
)

# Platform-specific
if(WIN32)
    target_compile_definitions(simulator PRIVATE _USE_MATH_DEFINES)
endif()

# Disable Arduino-specific things
target_compile_definitions(simulator PRIVATE
    SIMULATOR_BUILD
    PI=3.14159265358979323846
    TWO_PI=6.28318530717958647692
    HALF_PI=1.57079632679489661923
)
