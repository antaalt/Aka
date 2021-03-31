cmake_minimum_required (VERSION 3.10)
project(AkaMiniMp3)

add_library(AkaMiniMp3 INTERFACE)

target_include_directories(AkaMiniMp3
	INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)