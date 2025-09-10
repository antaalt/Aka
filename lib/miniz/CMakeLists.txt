cmake_minimum_required (VERSION 3.10)
project(miniz)

include(GNUInstallDirs)

add_library(miniz "src/miniz.c")

target_include_directories(miniz
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Install the library and its headers
install(TARGETS miniz
	EXPORT minizTargets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include)

# Generate and install *-targets.cmake 
install(EXPORT minizTargets
	FILE miniz-targets.cmake
	NAMESPACE miniz::
	DESTINATION share/miniz)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/miniz-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/miniz-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/miniz-config.cmake" DESTINATION share/miniz)