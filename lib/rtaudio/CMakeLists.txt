cmake_minimum_required (VERSION 3.10)
project(rtaudio)

include(GNUInstallDirs)

add_library (rtaudio "src/RtAudio.cpp")

if(WIN32)
	target_compile_definitions(rtaudio PRIVATE __WINDOWS_DS__)
elseif(UNIX)
	target_compile_definitions(rtaudio PRIVATE __LINUX_PULSE__)
	find_path(PULSEAUDIO_INCLUDE_DIR NAMES pulse/pulseaudio.h)
	find_library(PULSEAUDIO_LIBRARY NAMES pulse)
	target_include_directories(rtaudio PRIVATE ${PULSEAUDIO_INCLUDE_DIR})
	target_link_libraries (rtaudio PRIVATE pulse-simple pulse)
endif()

target_include_directories(rtaudio PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install the library and its headers
install(TARGETS rtaudio
	EXPORT rtaudioTargets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include)

# Generate and install *-targets.cmake 
install(EXPORT rtaudioTargets
	FILE rtaudio-targets.cmake
	NAMESPACE rtaudio::
	DESTINATION share/rtaudio)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rtaudio-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/rtaudio-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/rtaudio-config.cmake" DESTINATION share/rtaudio)