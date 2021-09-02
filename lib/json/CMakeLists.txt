cmake_minimum_required (VERSION 3.10)
project(AkaJson)

add_library(AkaJson INTERFACE)

target_include_directories(AkaJson
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
