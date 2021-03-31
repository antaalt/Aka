cmake_minimum_required (VERSION 3.10)
project(AkaMiniz)

add_library(AkaMiniz "src/miniz.c")

target_include_directories(AkaMiniz
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)