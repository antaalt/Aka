cmake_minimum_required (VERSION 3.10)
project(AkaRtAudio)

add_library (AkaRtAudio "src/RtAudio.cpp")

if(WIN32)
	target_compile_definitions(AkaRtAudio PRIVATE __WINDOWS_DS__)
elseif(UNIX)
	target_compile_definitions(AkaRtAudio PRIVATE __LINUX_PULSE__)
	find_path(PULSEAUDIO_INCLUDE_DIR NAMES pulse/pulseaudio.h)
	find_library(PULSEAUDIO_LIBRARY NAMES pulse)
	target_include_directories(AkaRtAudio PRIVATE ${PULSEAUDIO_INCLUDE_DIR})
	target_link_libraries (AkaRtAudio PRIVATE pulse-simple pulse)
endif()

target_include_directories(AkaRtAudio
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)