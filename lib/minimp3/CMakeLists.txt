cmake_minimum_required (VERSION 3.10)
project(minimp3)

include(GNUInstallDirs)

add_library(minimp3 INTERFACE)

target_include_directories(minimp3
	INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install the library and its headers
install(TARGETS minimp3
	EXPORT minimp3Targets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include)

# Generate and install *-targets.cmake 
install(EXPORT minimp3Targets
	FILE minimp3-targets.cmake
	NAMESPACE minimp3::
	DESTINATION share/minimp3)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/minimp3-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/minimp3-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/minimp3-config.cmake" DESTINATION share/minimp3)