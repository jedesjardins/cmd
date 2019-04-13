cmake_minimum_required(VERSION 3.12)
project(cmd)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CMD_BUILD_EXAMPLES "Build documentation examples" OFF)
option(CMD_BUILD_TESTS "Build documentation tests" OFF)

add_library(Cmd INTERFACE)
target_include_directories(Cmd
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

add_library(JED::cmd ALIAS Cmd)

if(CMD_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(CMD_BUILD_TESTS)
	add_subdirectory(tests)
endif()