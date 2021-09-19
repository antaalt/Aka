#pragma once

#include "Platform/Platform.h"
#include "Platform/PlatformBackend.h"
#include "Platform/Input.h"
#include "Platform/InputBackend.h"

#include "Core/Container/Vector.h"
#include "Core/Container/Array.h"
#include "Core/Container/String.h"
#include "Core/Container/Blob.h"
#include "Core/Geometry.h"
#include "Core/Encoding.h"
#include "Core/Application.h"
#include "Core/View.h"
#include "Core/Debug.h"
#include "Core/Font.h"
#include "Core/Sprite.h"
#include "Core/Physic.h"
#include "Core/Controller.h"
#include "Core/Event.h"

#include "Graphic/GraphicBackend.h"
#include "Graphic/Mesh.h"
#include "Graphic/Framebuffer.h"
#include "Graphic/Texture2D.h"
#include "Graphic/Texture2DMultisample.h"
#include "Graphic/TextureCubeMap.h"
#include "Graphic/Texture2DArray.h"
#include "Graphic/Sampler.h"
#include "Graphic/Device.h"
#include "Graphic/RenderPass.h"
#include "Graphic/Program.h"
#include "Graphic/Shader.h"

#include "Drawing/Batch2D.h"
#include "Drawing/Batch3D.h"
#include "Drawing/Renderer2D.h"
#include "Drawing/Renderer3D.h"

#include "Audio/Audio.h"
#include "Audio/AudioStream.h"
#include "Audio/AudioBackend.h"

#include "OS/Logger.h"
#include "OS/FileSystem.h"
#include "OS/Time.h"
#include "OS/Image.h"
#include "OS/Endian.h"
#include "OS/StopWatch.h"
#include "OS/Stream/MemoryStream.h"
#include "OS/Stream/FileStream.h"

#include "Scene/World.h"
#include "Scene/Entity.h"
#include "Scene/System.h"
#include "Scene/Component.h"

#include "Resource/Storage.h"
#include "Resource/TextureStorage.h"
#include "Resource/BufferStorage.h"
#include "Resource/MeshStorage.h"
#include "Resource/FontStorage.h"
#include "Resource/AudioStorage.h"
#include "Resource/ProgramManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourceAllocator.h"
#include "Resource/ResourceManager.h"


namespace aka {

};