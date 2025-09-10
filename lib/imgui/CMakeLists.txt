cmake_minimum_required (VERSION 3.10)
project(AkaImGui)

set(SRC "")
set(LIBS "")
set(INCLUDE_DIRS "")
set(COMPILES "")

if(GRAPHIC_OPENGL)
	set(SRC
		"${SRC}"
		"src/backends/imgui_impl_opengl3.cpp"
	)
	set(LIBS OpenGL::GL GLEW::GLEW)
	set(INCLUDE_DIRS "${GLEW_INCLUDE_DIRS}")
	set(COMPILES "IMGUI_IMPL_OPENGL_LOADER_GLEW")
elseif(GRAPHIC_D3D11)
	set(SRC
		"${SRC}"
		"src/backends/imgui_impl_dx11.cpp"
	)
elseif(GRAPHIC_VULKAN)
	set(SRC
		"${SRC}"
		"src/backends/imgui_impl_vulkan.cpp"
	)
	set(LIBS Vulkan::Vulkan)
	set(INCLUDE_DIRS "${Vulkan_INCLUDE_DIRS}")
else()
	message(WARNING "ImGui not set. Disable layer.")
endif()

if(PLATFORM_GLFW3)
	set(SRC
		"${SRC}"
		"src/backends/imgui_impl_glfw.cpp"
	)
	set(LIBS glfw)
endif()

add_library(AkaImGui
	"src/imgui.cpp"
	"src/imgui_demo.cpp"
	"src/imgui_draw.cpp"
	"src/imgui_tables.cpp"
	"src/imgui_widgets.cpp"
	"src/imguizmo.cpp"
	"${SRC}"
)
target_include_directories(AkaImGui PRIVATE ${INCLUDE_DIRS})
target_compile_definitions(AkaImGui PUBLIC AKA_USE_IMGUI_LAYER ${COMPILES})
target_link_libraries(AkaImGui PRIVATE ${LIBS})

target_include_directories(AkaImGui
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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