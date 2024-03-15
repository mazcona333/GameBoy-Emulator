cmake_minimum_required(VERSION 3.14)
project(gba_emulator)

set(CMAKE_CXX_STANDARD 11)

set(SDL2_DIR ../../SDL2-2.24.0/cmake)

find_package(SDL2 REQUIRED)

# set(SOURCE_FILES src/test/autoTest.cpp src/private/gba.cpp src/private/cpu.cpp src/private/cpuDebug.cpp src/private/ppu.cpp src/private/memory.cpp src/nlohmann/json.hpp)
set(SOURCE_FILES src/main.cpp src/private/gba.cpp src/private/cpu.cpp src/private/cpuDebug.cpp src/private/ppu.cpp src/private/memory.cpp src/private/platform.cpp)

add_executable(
	gba_emulator
	${SOURCE_FILES})

target_link_libraries(gba_emulator PRIVATE SDL2::SDL2)