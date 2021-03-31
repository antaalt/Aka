cmake_minimum_required (VERSION 3.10)
project(AkaStb)

add_library(AkaStb INTERFACE)

target_include_directories(AkaStb
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)