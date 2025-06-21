cmake_minimum_required (VERSION 3.10)
project(Aka)

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

# Disable SPIRV opt
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)

# Third party libs
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/freetype)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/glslang)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SPIRV-Cross)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/geometry)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/utf8)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/miniz)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/minimp3)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/stb)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/json)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/lua)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/renderdoc)

target_link_libraries(Aka PUBLIC
	Geometry
	AkaImGui
	AkaLua
)
target_link_libraries(Aka PRIVATE
	AkaMiniz
	AkaUtfCpp
	AkaRenderdoc
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
