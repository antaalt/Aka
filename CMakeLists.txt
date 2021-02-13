cmake_minimum_required (VERSION 3.10)
project(Aka)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ajoutez une source à l'exécutable de ce projet.
add_library (Aka
	"src/Audio/AudioBackend.cpp"
	"src/Audio/RtAudioBackend.cpp"
	"src/Audio/AudioStream.cpp" 
	"src/Audio/AudioStreamMp3.cpp" 
	"src/Audio/AudioStreamMemory.cpp"

	"src/Core/Application.cpp"
	"src/Core/Debug.cpp"
	"src/Core/Font.cpp"
	"src/Core/Physic.cpp"
	"src/Core/Aseprite.cpp"
	"src/Core/Sprite.cpp"

	"src/Scene/Component.cpp"
	"src/Scene/Entity.cpp"
	"src/Scene/System.cpp"
	"src/Scene/World.cpp"
	"src/Scene/Serializer.cpp"

	"src/Graphic/D3D11/D3D11Backend.cpp"
	"src/Graphic/GL/GLBackend.cpp"
	"src/Graphic/Batch.cpp"
	"src/Graphic/Device.cpp"
	"src/Graphic/Framebuffer.cpp"
	"src/Graphic/Mesh.cpp"
	"src/Graphic/RenderPass.cpp"
	"src/Graphic/Shader.cpp"
	"src/Graphic/ShaderMaterial.cpp"
	"src/Graphic/Texture.cpp"

	"src/OS/FileSystem.cpp"
	"src/OS/Image.cpp"
	"src/OS/ImagePacker.cpp"
	"src/OS/Logger.cpp"
	"src/OS/Time.cpp"

	"src/Platform/Input.cpp"
	"src/Platform/PlatformGLFW3.cpp"
	"src/Platform/PlatformLinux.cpp"
	"src/Platform/PlatformWindows.cpp"

	"lib/rtaudio/RtAudio.cpp"
	"lib/miniz/miniz.c"
)

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

# Libs
set(LIBS "")

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
	if (GLEW_FOUND)
		include_directories(${GLEW_INCLUDE_DIRS})
		target_link_libraries(Aka PRIVATE OpenGL::GL GLEW::GLEW)
	endif()
elseif (GRAPHIC_D3D11)
	# Setup D3D11
	target_compile_definitions(Aka PUBLIC AKA_USE_D3D11)
	# D3D11 libs are loaded with #pragma within the code
endif()

# Link RtAudio
if (AUDIO_RTAUDIO)
	target_compile_definitions(Aka PUBLIC AKA_USE_RTAUDIO)
if(WIN32)
	target_compile_definitions(Aka PRIVATE __WINDOWS_DS__)
elseif(UNIX)
	target_compile_definitions(Aka PRIVATE __LINUX_ALSA__)
	find_package(ALSA REQUIRED)                                    
	if (ALSA_FOUND)
		include_directories(${ALSA_INCLUDE_DIRS})
		target_link_libraries (Aka PRIVATE ${ALSA_LIBRARIES})
	endif(ALSA_FOUND)
endif()
	target_include_directories(Aka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/rtaudio)
endif()

# Link GLFW3
if (PLATFORM_GLFW3)
	target_compile_definitions(Aka PUBLIC AKA_USE_GLFW3)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	add_subdirectory("lib/glfw/")
	target_link_libraries(Aka PRIVATE glfw)
endif()

# Third party libs
# Geometry
add_subdirectory(lib/geometry)
# Freetype
add_subdirectory(lib/freetype)
target_include_directories(Aka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/freetype/include)
target_link_libraries(Aka PRIVATE freetype)
# UTF8
target_include_directories(Aka PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/utf8)
# STB
target_include_directories(Aka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb)
# minimp3
target_include_directories(Aka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/minimp3)
# miniz
target_include_directories(Aka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/miniz)
# entt
target_include_directories(Aka PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/entt)

# Enable UNICODE in windows
if(WIN32)
	target_compile_definitions(Aka PUBLIC UNICODE)
endif()
# Ensure UTF8 at runtime
if (MSVC)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()
