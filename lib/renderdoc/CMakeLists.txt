cmake_minimum_required (VERSION 3.10)
project(AkaRenderdoc)

add_library(AkaRenderdoc INTERFACE)

target_include_directories(AkaRenderdoc
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
