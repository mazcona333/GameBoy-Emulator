cmake_minimum_required(VERSION 3.14)
project(gb_emulator)

set(CMAKE_CXX_STANDARD 11)

set(SDL2_DIR ../../SDL2-2.24.0/cmake)

find_package(SDL2 REQUIRED)

# set(SOURCE_FILES src/test/autoTest.cpp src/private/gb.cpp src/private/cpu.cpp src/private/cpuDebug.cpp src/private/ppu.cpp src/private/memory.cpp src/private/platform.cpp)
set(SOURCE_FILES src/main.cpp src/private/gb.cpp src/private/cpu.cpp src/private/cpuDebug.cpp src/private/ppu.cpp src/private/memory.cpp src/private/mbc.cpp src/private/mbc0.cpp src/private/mbc1.cpp src/private/mbc2.cpp src/private/mbc3.cpp src/private/platform.cpp)

add_executable(
	gb_emulator
	${SOURCE_FILES})

target_link_libraries(gb_emulator PRIVATE SDL2::SDL2)