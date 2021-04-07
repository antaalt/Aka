cmake_minimum_required (VERSION 3.10)
project(AkaImGui)


set(SRC "")
set(LIBS "")
set(INCLUDE_DIRS "")
set(COMPILES "")
set(ENABLED FALSE)

if(GRAPHIC_OPENGL)
	set(SRC 
		"${SRC}"
		"src/backends/imgui_impl_opengl3.cpp"
	)
	set(LIBS OpenGL::GL GLEW::GLEW)
	set(INCLUDE_DIRS "${GLEW_INCLUDE_DIRS}")
	set(COMPILES "IMGUI_IMPL_OPENGL_LOADER_GLEW")
	set(ENABLED TRUE)
elseif(GRAPHIC_D3D11)
	set(SRC 
		"${SRC}"
		"src/backends/imgui_impl_dx11.cpp"
	)
	set(ENABLED TRUE)
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

if(ENABLED)
	add_library(AkaImGui 
		"src/imgui.cpp"
		"src/imgui_demo.cpp"
		"src/imgui_draw.cpp"
		"src/imgui_tables.cpp"
		"src/imgui_widgets.cpp"
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
endif()