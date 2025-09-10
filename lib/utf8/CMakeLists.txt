cmake_minimum_required (VERSION 3.10)
project(AkaUtfCpp)

include(GNUInstallDirs)

add_library(AkaUtfCpp INTERFACE)

target_include_directories(AkaUtfCpp
	INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install the library and its headers
install(TARGETS AkaUtfCpp
	EXPORT AkaUtfCppTargets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include)

# Generate and install *-targets.cmake 
install(EXPORT AkaUtfCppTargets
	FILE utfpp-targets.cmake
	NAMESPACE utfpp::
	DESTINATION share/utfpp)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/utfpp-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/utfpp-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/utfpp-config.cmake" DESTINATION share/utfpp)