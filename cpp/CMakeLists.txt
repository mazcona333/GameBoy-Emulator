cmake_minimum_required(VERSION 3.14)
project(gba_emulator)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/test.cpp src/gba.cpp src/cpu.cpp)

add_executable(
	gba_emulator
	${SOURCE_FILES})