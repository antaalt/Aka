cmake_minimum_required (VERSION 3.19)
project(Aka)

include(GNUInstallDirs)

# Ajoutez une source à l'exécutable de ce projet.
add_library (Aka
	"src/Audio/AudioDevice.cpp"
	"src/Audio/AudioStream.cpp"
	"src/Audio/AudioStreamMp3.cpp"
	"src/Audio/AudioStreamMemory.cpp"
	"src/Audio/RtAudio/RtAudioDevice.cpp"

	"src/Core/Container/Blob.cpp"
	"src/Core/Container/String.cpp"
	"src/Core/Application.cpp"
	"src/Core/Debug.cpp"
	"src/Core/Physic.cpp"
	"src/Core/Encoding.cpp"
	"src/Core/Controller.cpp"
	"src/Core/Config.cpp" 
	"src/Core/BitField.cpp"
	"src/Core/Bitmask.cpp"
	"src/Core/Layer.cpp"
	"src/Core/Worker/WorkerPool.cpp" 
	"src/Core/Worker/Worker.cpp"
	"src/Core/Worker/Job.cpp"
	"src/Core/Hash.cpp"

	"src/Memory/Allocator.cpp"
	"src/Memory/Memory.cpp"
	"src/Memory/Allocator/DefaultAllocator.cpp"
	"src/Memory/Allocator/PoolAllocator.cpp"
	"src/Memory/Allocator/LinearAllocator.cpp"
	"src/Memory/AllocatorTracker.cpp"

	"src/Layer/ImGuiLayer.cpp"

	"src/Renderer/Renderer.cpp"
	"src/Renderer/Instance.cpp"
	"src/Renderer/InstanceRenderer.cpp" 
	"src/Renderer/InstanceRenderer/StaticMeshInstanceRenderer.cpp" 
	"src/Renderer/InstanceRenderer/SkeletalMeshInstanceRenderer.cpp" 
	"src/Renderer/Instance/StaticMeshInstance.cpp" 
	"src/Renderer/Instance/SkeletalMeshInstance.cpp"
	"src/Renderer/DebugDraw/DebugDrawList.cpp"

	"src/Scene/Component.cpp"
	"src/Scene/Node.cpp"
	"src/Scene/NodeAllocator.cpp"
	"src/Scene/ComponentAllocator.cpp"
	"src/Scene/Component/CameraComponent.cpp"
	"src/Scene/Component/ArcballComponent.cpp"
	"src/Scene/Component/StaticMeshComponent.cpp"
	"src/Scene/Component/SkeletalMeshComponent.cpp"
	"src/Scene/Component/RigidBodyComponent.cpp"
	"src/Scene/Component/ColliderComponent.cpp"

	"src/Resource/Archive/ArchiveGeometry.cpp"
	"src/Resource/Archive/ArchiveMaterial.cpp"
	"src/Resource/Archive/ArchiveBatch.cpp"
	"src/Resource/Archive/ArchiveStaticMesh.cpp"
	"src/Resource/Archive/ArchiveSkeletalMesh.cpp"
	"src/Resource/Archive/ArchiveSkeletonAnimation.cpp"
	"src/Resource/Archive/ArchiveSkeleton.cpp"
	"src/Resource/Archive/ArchiveImage.cpp"
	"src/Resource/Archive/ArchiveScene.cpp"
	"src/Resource/Archive/ArchiveSprite.cpp"
	"src/Resource/Archive/ArchiveFont.cpp"
	"src/Resource/Archive/Archive.cpp"
	"src/Resource/Resource/Resource.cpp"
	"src/Resource/Resource/Scene.cpp"
	"src/Resource/Resource/Sprite.cpp"
	"src/Resource/Resource/StaticMesh.cpp"
	"src/Resource/Resource/SkeletalMesh.cpp"
	"src/Resource/Resource/Texture.cpp"
	"src/Resource/Resource/Font.cpp"
	"src/Resource/Resource/Material.cpp"
	"src/Resource/Importer/Importer.cpp"
	"src/Resource/Importer/AsepriteImporter.cpp"
	"src/Resource/Importer/TextureImporter.cpp"
	"src/Resource/Importer/FontImporter.cpp"
	"src/Resource/Shader/Shader.cpp"
	"src/Resource/Shader/ShaderRegistry.cpp" 
	"src/Resource/Shader/ShaderCompiler.cpp"
	"src/Resource/Asset.cpp"
	"src/Resource/AssetLibrary.cpp"
	"src/Resource/AssetPath.cpp"

	
	"src/Graphic/Vulkan/VulkanGraphicDevice.cpp"
	"src/Graphic/Vulkan/VulkanSwapchain.cpp"
	"src/Graphic/Vulkan/VulkanBuffer.cpp"
	"src/Graphic/Vulkan/VulkanCommandList.cpp"
	"src/Graphic/Vulkan/VulkanTexture.cpp"
	"src/Graphic/Vulkan/VulkanFramebuffer.cpp"
	"src/Graphic/Vulkan/VulkanProgram.cpp"
	"src/Graphic/Vulkan/VulkanPipeline.cpp"
	"src/Graphic/Vulkan/VulkanSampler.cpp"
	"src/Graphic/Vulkan/VulkanDebug.cpp"
	"src/Graphic/Vulkan/VulkanRenderPass.cpp"
	"src/Graphic/Vulkan/VulkanFence.cpp"
	"src/Graphic/Vulkan/VulkanBarrier.cpp"
	"src/Graphic/Vulkan/VulkanInstance.cpp"
	"src/Graphic/Vulkan/VulkanCommon.cpp"
	"src/Graphic/Vulkan/Extensions/VkDebugUtils.cpp"
	"src/Graphic/Vulkan/Extensions/VkMeshShader.cpp"
	"src/Graphic/Resource.cpp"
	"src/Graphic/Program.cpp"
	"src/Graphic/GraphicDevice.cpp"
	"src/Graphic/Framebuffer.cpp"
	"src/Graphic/Shader.cpp"
	"src/Graphic/Texture.cpp"
	"src/Graphic/Pipeline.cpp"
	"src/Graphic/Sampler.cpp"
	"src/Graphic/Buffer.cpp"
	"src/Graphic/DescriptorSet.cpp"
	"src/Graphic/RenderPass.cpp" 
	"src/Graphic/Fence.cpp"
	"src/Graphic/DescriptorPool.cpp" 
	"src/Graphic/PhysicalDevice.cpp"
	"src/Graphic/Swapchain.cpp" 
	"src/Graphic/Surface.cpp"
	"src/Graphic/Instance.cpp"

	"src/OS/Path.cpp"
	"src/OS/Image.cpp"
	"src/OS/Packer.cpp"
	"src/OS/Logger.cpp"
	"src/OS/Time.cpp"
	"src/OS/Endian.cpp"
	"src/OS/Stream/Stream.cpp"
	"src/OS/Stream/MemoryStream.cpp"
	"src/OS/Stream/FileStream.cpp"
	"src/OS/Archive.cpp"
	"src/OS/OS.cpp"
	"src/OS/Windows/OSWindows.cpp"
	"src/OS/Windows/FileWatcherWindows.cpp"
	"src/OS/Linux/OSLinux.cpp"
	"src/OS/Linux/FileWatcherINotify.cpp"

	"src/Platform/Input.cpp"
	"src/Platform/GLFW3/PlatformGLFW3.cpp"
	"src/Platform/PlatformDevice.cpp"
	"src/Platform/PlatformWindow.cpp"
	"src/Platform/PlatformView.cpp"
)

target_compile_features(Aka PUBLIC cxx_std_17)

target_include_directories(Aka
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Platform Backend
set(PLATFORM_GLFW3 true CACHE BOOL "Set GLFW3 as platform backend")
# Graphic Backend
set(GRAPHIC_VULKAN true CACHE BOOL "Set Vulkan as graphic backend")
set(GRAPHIC_D3D12 false CACHE BOOL "Set DirectX12 as graphic backend")
# Audio Backend
set(AUDIO_RTAUDIO true CACHE BOOL "Set RtAudio as audio backend")

# TODO: Handle features to optionally toggle on off vulkan and others via installation.
if (GRAPHIC_VULKAN)
	# Setup Vulkan
	target_compile_definitions(Aka PUBLIC AKA_USE_VULKAN)
	find_package(Vulkan REQUIRED) # Use VULKAN_SDK env to find Vulkan
	target_include_directories(Aka PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_link_libraries(Aka PRIVATE Vulkan::Vulkan)
elseif (GRAPHIC_D3D12)
	# Setup D3D12
endif()

# Link RtAudio
if (AUDIO_RTAUDIO)
	target_compile_definitions(Aka PUBLIC AKA_USE_RTAUDIO)
	find_package(rtaudio REQUIRED PATHS lib/install/rtaudio NO_DEFAULT_PATH)
	target_link_libraries(Aka PRIVATE rtaudio::rtaudio)
endif()

# Link GLFW3
if (PLATFORM_GLFW3)
	target_compile_definitions(Aka PUBLIC AKA_USE_GLFW3)
	find_package(glfw3 REQUIRED PATHS lib/install/glfw NO_DEFAULT_PATH)
	target_link_libraries(Aka PRIVATE glfw)
endif()

# Freetype
find_package(freetype REQUIRED PATHS lib/install/freetype NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE freetype)
# Glslang
find_package(glslang REQUIRED PATHS lib/install/glslang NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE glslang)
# Imgui
find_package(imgui REQUIRED PATHS lib/install/imgui NO_DEFAULT_PATH)
target_link_libraries(Aka PUBLIC imgui) # Let user use it.
# Miniz
find_package(miniz REQUIRED PATHS lib/install/miniz NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE miniz::miniz)
# Minimp3
find_package(minimp3 REQUIRED PATHS lib/install/minimp3 NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE minimp3::minimp3)
# Stb
find_package(stb REQUIRED PATHS lib/install/stb NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE stb::AkaStb)
# Nlohmann JSON
find_package(nlohmann-json REQUIRED PATHS lib/install/json NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE nlohmann-json::AkaJson)
# SPIRV Cross
# Do we really need to load these three...
find_package(spirv_cross_core REQUIRED PATHS lib/install/SPIRV-Cross NO_DEFAULT_PATH)
find_package(spirv_cross_glsl REQUIRED PATHS lib/install/SPIRV-Cross NO_DEFAULT_PATH)
find_package(spirv_cross_cpp REQUIRED PATHS lib/install/SPIRV-Cross NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE spirv_cross_core spirv_cross_glsl spirv_cross_cpp)
# UTF8 CPP
# Not found in vcpkg registry...
find_package(utfpp REQUIRED PATHS lib/install/utf8 NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE utfpp::AkaUtfCpp)
# Renderdoc
# Not found in vcpkg registry...
find_package(renderdoc REQUIRED PATHS lib/install/renderdoc NO_DEFAULT_PATH)
target_link_libraries(Aka PRIVATE renderdoc::AkaRenderdoc)
# Geometry
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/geometry)
target_link_libraries(Aka PUBLIC Geometry)


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

# Install the library and its headers
install(TARGETS Aka Geometry
	EXPORT AkaTargets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Generate and install *-targets.cmake 
install(EXPORT AkaTargets
	FILE aka-targets.cmake
	NAMESPACE aka::
	DESTINATION share/aka)

# Generate the config file in the current binary dir (this ensures it's not placed directly in source)
# TODO: Cant we generate this automatically ? Hardcoded pass for now.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/aka-config.cmake"
"include(CMakeFindDependencyMacro)\n"
# TODO: need conditions aswell here ?
"find_package(Vulkan REQUIRED)\n"
"find_package(rtaudio REQUIRED PATHS lib/Aka/lib/install/rtaudio NO_DEFAULT_PATH)\n"
"find_package(glfw3 REQUIRED PATHS lib/Aka/lib/install/glfw NO_DEFAULT_PATH)\n"
"find_package(freetype REQUIRED PATHS lib/Aka/lib/install/freetype NO_DEFAULT_PATH)\n"
"find_package(glslang REQUIRED PATHS lib/Aka/lib/install/glslang NO_DEFAULT_PATH)\n"
"find_package(imgui REQUIRED PATHS lib/Aka/lib/install/imgui NO_DEFAULT_PATH)\n"
"find_package(miniz REQUIRED PATHS lib/Aka/lib/install/miniz NO_DEFAULT_PATH)\n"
"find_package(minimp3 REQUIRED PATHS lib/Aka/lib/install/minimp3 NO_DEFAULT_PATH)\n"
"find_package(stb REQUIRED PATHS lib/Aka/lib/install/stb NO_DEFAULT_PATH)\n"
"find_package(nlohmann-json REQUIRED PATHS lib/Aka/lib/install/json NO_DEFAULT_PATH)\n"
"find_package(spirv_cross_core REQUIRED PATHS lib/Aka/lib/install/SPIRV-Cross NO_DEFAULT_PATH)\n"
"find_package(spirv_cross_glsl REQUIRED PATHS lib/Aka/lib/install/SPIRV-Cross NO_DEFAULT_PATH)\n"
"find_package(spirv_cross_cpp REQUIRED PATHS lib/Aka/lib/install/SPIRV-Cross NO_DEFAULT_PATH)\n"
"find_package(utfpp REQUIRED PATHS lib/Aka/lib/install/utf8 NO_DEFAULT_PATH)\n"
"find_package(renderdoc REQUIRED PATHS lib/Aka/lib/install/renderdoc NO_DEFAULT_PATH)\n"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/aka-targets.cmake\")\n"
)

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/aka-config.cmake"
	DESTINATION share/aka)