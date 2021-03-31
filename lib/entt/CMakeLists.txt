cmake_minimum_required (VERSION 3.10)
project(AkaEntt)

add_library(AkaEntt INTERFACE)

target_include_directories(AkaEntt
	INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)