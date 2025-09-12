cmake_minimum_required (VERSION 3.10)
project(AkaImGui)

include(GNUInstallDirs)

add_library(AkaImGui
	"src/imgui.cpp"
	"src/imgui_demo.cpp"
	"src/imgui_draw.cpp"
	"src/imgui_tables.cpp"
	"src/imgui_widgets.cpp"
	"src/imguizmo.cpp"
	"src/backends/imgui_impl_glfw.cpp"
	"src/backends/imgui_impl_vulkan.cpp"
)
target_compile_definitions(AkaImGui PUBLIC AKA_USE_IMGUI_LAYER)

# Link glfw
find_package(glfw3 REQUIRED PATHS ../install/glfw NO_DEFAULT_PATH)
target_link_libraries(AkaImGui PRIVATE glfw)
# Link Vulkan
find_package(Vulkan REQUIRED) # Use VULKAN_SDK env to find Vulkan
target_include_directories(AkaImGui PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(AkaImGui PRIVATE Vulkan::Vulkan)

target_include_directories(AkaImGui
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/include/backends>"
)

# Install the library and its headers
install(TARGETS AkaImGui
	EXPORT AkaImGuiTargets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include)

# Generate and install *-targets.cmake 
install(EXPORT AkaImGuiTargets
	FILE imgui-targets.cmake
	NAMESPACE imgui::
	DESTINATION share/imgui)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/imgui-config.cmake"
"include(CMakeFindDependencyMacro)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/imgui-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/imgui-config.cmake" DESTINATION share/imgui)