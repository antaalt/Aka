﻿cmake_minimum_required (VERSION 3.10)
project(Aka)

# Ajoutez une source à l'exécutable de ce projet.
add_library (Aka
	"src/Audio/AudioDevice.cpp"
	"src/Audio/AudioStream.cpp"
	"src/Audio/AudioStreamMp3.cpp"
	"src/Audio/AudioStreamMemory.cpp"
	"src/Audio/RtAudio/AudioRtAudio.cpp"

	"src/Core/Container/Blob.cpp"
	"src/Core/Container/String.cpp"
	"src/Core/Application.cpp"
	"src/Core/View.cpp"
	"src/Core/Debug.cpp"
	"src/Core/Font.cpp"
	"src/Core/Physic.cpp"
	"src/Core/Aseprite.cpp"
	"src/Core/Sprite.cpp"
	"src/Core/Encoding.cpp"
	"src/Core/Controller.cpp"

	"src/Memory/Allocator.cpp"
	"src/Memory/Memory.cpp"
	"src/Memory/Allocator/PoolAllocator.cpp" 
	"src/Memory/Allocator/LinearAllocator.cpp"

	"src/Layer/ImGuiLayer.cpp"

	"src/Scene/Component.cpp"
	"src/Scene/Entity.cpp"
	"src/Scene/System.cpp"
	"src/Scene/World.cpp"
	"src/Scene/Serializer.cpp"
	"src/Scene/Camera.cpp"

	"src/Resource/ProgramManager.cpp"
	"src/Resource/ResourceManager.cpp"
	"src/Resource/MeshStorage.cpp"
	"src/Resource/FontStorage.cpp"
	"src/Resource/AudioStorage.cpp"
	"src/Resource/TextureStorage.cpp"
	"src/Resource/BufferStorage.cpp"

	"src/Drawing/Renderer2D.cpp"
	"src/Drawing/Renderer3D.cpp"
	"src/Drawing/Batch2D.cpp"
	"src/Drawing/Batch3D.cpp"

	"src/Graphic/D3D11/D3D11Context.cpp"
	"src/Graphic/D3D11/D3D11Device.cpp"
	"src/Graphic/D3D11/D3D11Backbuffer.cpp"
	"src/Graphic/D3D11/D3D11Buffer.cpp"
	"src/Graphic/D3D11/D3D11Texture.cpp"
	"src/Graphic/D3D11/D3D11Mesh.cpp"
	"src/Graphic/D3D11/D3D11Material.cpp"
	"src/Graphic/D3D11/D3D11Framebuffer.cpp"
	"src/Graphic/D3D11/D3D11Program.cpp"
	"src/Graphic/D3D11/Internal/D3D11Blit.cpp"

	"src/Graphic/GL/GLContext.cpp"
	"src/Graphic/GL/GLDevice.cpp"
	"src/Graphic/GL/GLBackbuffer.cpp"
	"src/Graphic/GL/GLBuffer.cpp"
	"src/Graphic/GL/GLTexture.cpp"
	"src/Graphic/GL/GLMesh.cpp"
	"src/Graphic/GL/GLMaterial.cpp"
	"src/Graphic/GL/GLFramebuffer.cpp"
	"src/Graphic/GL/GLProgram.cpp"

	"src/Graphic/GraphicDevice.cpp"
	"src/Graphic/Device.cpp"
	"src/Graphic/Framebuffer.cpp"
	"src/Graphic/Backbuffer.cpp"
	"src/Graphic/Mesh.cpp"
	"src/Graphic/RenderPass.cpp"
	"src/Graphic/Compiler.cpp"
	"src/Graphic/Shader.cpp"
	"src/Graphic/Program.cpp"
	"src/Graphic/Material.cpp"
	"src/Graphic/Texture.cpp"
	"src/Graphic/Texture2D.cpp"
	"src/Graphic/Texture2DMultisample.cpp"
	"src/Graphic/TextureCubeMap.cpp"
	"src/Graphic/Texture2DArray.cpp"
	"src/Graphic/Sampler.cpp"
	"src/Graphic/Buffer.cpp"

	"src/OS/Path.cpp"
	"src/OS/Image.cpp"
	"src/OS/Packer.cpp"
	"src/OS/Logger.cpp"
	"src/OS/Time.cpp"
	"src/OS/Endian.cpp"
	"src/OS/Stream/MemoryStream.cpp"
	"src/OS/Stream/FileStream.cpp"
	"src/OS/OS.cpp"

	"src/OS/Windows/OSWindows.cpp"

	"src/OS/Linux/OSLinux.cpp" 

	"src/Platform/Input.cpp"
	"src/Platform/GLFW3/PlatformGLFW3.cpp" 
	"src/Platform/PlatformDevice.cpp" 
)

target_compile_features(Aka PUBLIC cxx_std_17)

target_include_directories(Aka
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Platform Backend
set(PLATFORM_GLFW3 true CACHE BOOL "Set GLFW3 as platform backend")
# Graphic Backend
set(GRAPHIC_OPENGL true CACHE BOOL "Set OpenGL as graphic backend")
set(GRAPHIC_D3D11 false CACHE BOOL "Set DirectX11 as graphic backend")
# Audio Backend
set(AUDIO_RTAUDIO true CACHE BOOL "Set RtAudio as audio backend")

if (GRAPHIC_OPENGL)
	# Setup OpenGL and glew
	target_compile_definitions(Aka PUBLIC AKA_USE_OPENGL)
	# GL
	if(UNIX)
		set(OpenGL_GL_PREFERENCE "GLVND")
	endif(UNIX)
	find_package(OpenGL REQUIRED)
	# Glew
	if(MSVC)
		set(GLEW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/glew")
		set(CMAKE_PREFIX_PATH "${GLEW_DIR}")
		set(CMAKE_LIBRARY_PATH "${GLEW_DIR}/lib/")
	endif()
	find_package(GLEW 2.1 REQUIRED)
	target_include_directories(Aka PRIVATE ${GLEW_INCLUDE_DIRS})
	target_link_libraries(Aka PRIVATE OpenGL::GL GLEW::GLEW)
elseif (GRAPHIC_D3D11)
	# Setup D3D11
	target_compile_definitions(Aka PUBLIC AKA_USE_D3D11)
	# D3D11 libs are loaded with #pragma within the code
endif()

# Link RtAudio
if (AUDIO_RTAUDIO)
	target_compile_definitions(Aka PUBLIC AKA_USE_RTAUDIO)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/rtaudio)
	target_link_libraries(Aka PRIVATE AkaRtAudio)
endif()

# Link GLFW3
if (PLATFORM_GLFW3)
	target_compile_definitions(Aka PUBLIC AKA_USE_GLFW3)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/)
	target_link_libraries(Aka PRIVATE glfw)
endif()

# Third party libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/freetype)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/glslang)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SPIRV-Cross)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/geometry)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/utf8)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/miniz)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/minimp3)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/entt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/stb)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/json)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/lua)

target_link_libraries(Aka PUBLIC
	Geometry
	AkaEntt
	AkaImGui
	AkaLua
)
target_link_libraries(Aka PRIVATE
	AkaMiniz
	AkaUtfCpp
	AkaMiniMp3
	AkaStb
	AkaJson
	freetype
	glslang
	SPIRV
	spirv-cross-core
	spirv-cross-cpp
	spirv-cross-glsl
	spirv-cross-hlsl
)

# Enable UNICODE in windows
if(WIN32)
	target_compile_definitions(Aka PUBLIC UNICODE)
endif()
if (MSVC)
	# Ensure __cplusplus correctly set
	target_compile_options(Aka PUBLIC "/Zc:__cplusplus")
	# Ensure UTF8 at runtime
	target_compile_options(Aka PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	target_compile_options(Aka PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()
