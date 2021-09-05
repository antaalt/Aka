cmake_minimum_required (VERSION 3.10)
project(Aka)

# Ajoutez une source à l'exécutable de ce projet.
add_library (Aka
	"src/Audio/RtAudioBackend.cpp"
	"src/Audio/AudioStream.cpp"
	"src/Audio/AudioStreamMp3.cpp"
	"src/Audio/AudioStreamMemory.cpp"

	"src/Core/Application.cpp"
	"src/Core/View.cpp"
	"src/Core/Debug.cpp"
	"src/Core/Font.cpp"
	"src/Core/Physic.cpp"
	"src/Core/Aseprite.cpp"
	"src/Core/Sprite.cpp"
	"src/Core/String.cpp"
	"src/Core/Encoding.cpp"
	"src/Core/Controller.cpp"

	"src/Layer/ImGuiLayer.cpp"

	"src/Scene/Component.cpp"
	"src/Scene/Entity.cpp"
	"src/Scene/System.cpp"
	"src/Scene/World.cpp"
	"src/Scene/Serializer.cpp"
	"src/Scene/Camera.cpp"

	"src/Resource/ResourceManager.cpp"
	"src/Resource/MeshStorage.cpp"
	"src/Resource/FontStorage.cpp"
	"src/Resource/AudioStorage.cpp"
	"src/Resource/TextureStorage.cpp"
	"src/Resource/BufferStorage.cpp"

	"src/Graphic/D3D11/D3D11Backend.cpp"
	"src/Graphic/GL/GLBackend.cpp"
	"src/Drawing/Renderer2D.cpp"
	"src/Drawing/Renderer3D.cpp"
	"src/Drawing/Batch2D.cpp"
	"src/Drawing/Batch3D.cpp"
	"src/Graphic/Device.cpp"
	"src/Graphic/Framebuffer.cpp"
	"src/Graphic/Mesh.cpp"
	"src/Graphic/RenderPass.cpp"
	"src/Graphic/Shader.cpp"
	"src/Graphic/ShaderMaterial.cpp"
	"src/Graphic/Texture.cpp"
	"src/Graphic/Buffer.cpp"

	"src/OS/FileSystem.cpp"
	"src/OS/Image.cpp"
	"src/OS/ImagePacker.cpp"
	"src/OS/Logger.cpp"
	"src/OS/Time.cpp"
	"src/OS/Endian.cpp"

	"src/OS/Stream/MemoryStream.cpp"
	"src/OS/Stream/FileStream.cpp"

	"src/Platform/Input.cpp"
	"src/Platform/PlatformGLFW3.cpp"

	"src/Platform/Windows/WindowsPlatform.h"
	"src/Platform/Windows/WindowsFileSystem.cpp"
	"src/Platform/Windows/WindowsPlatform.cpp"
	"src/Platform/Linux/LinuxFileSystem.cpp"
	"src/Platform/Linux/LinuxPlatform.cpp"
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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/geometry)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/utf8)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/miniz)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/minimp3)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/entt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/stb)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/json)

target_link_libraries(Aka PUBLIC Geometry AkaEntt AkaImGui)
target_link_libraries(Aka PRIVATE AkaMiniz AkaUtfCpp AkaMiniMp3 AkaStb freetype AkaJson)

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
