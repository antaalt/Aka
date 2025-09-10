cmake_minimum_required (VERSION 3.10)
project(AkaRenderdoc)

include(GNUInstallDirs)

add_library(AkaRenderdoc INTERFACE)

target_include_directories(AkaRenderdoc
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install the library and its headers
install(TARGETS AkaRenderdoc
	EXPORT AkaRenderdocTargets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include)

# Generate and install *-targets.cmake 
install(EXPORT AkaRenderdocTargets
	FILE renderdoc-targets.cmake
	NAMESPACE renderdoc::
	DESTINATION share/renderdoc)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/renderdoc-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/renderdoc-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/renderdoc-config.cmake" DESTINATION share/renderdoc)