cmake_minimum_required (VERSION 3.10)
project(AkaUtfCpp)

add_library(AkaUtfCpp INTERFACE)

target_include_directories(AkaUtfCpp
	INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)