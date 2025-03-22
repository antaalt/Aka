#include <Aka/OS/Image.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/OS.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/BitField.h>
#include <Aka/Memory/Allocator.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8

struct StbAllocation { uint8_t data; };
static_assert(sizeof(StbAllocation) == 1);
#define STBI_MALLOC(sz)           (::aka::mem::getAllocator(::aka::AllocatorMemoryType::Object, ::aka::AllocatorCategory::Global).allocate<StbAllocation>(sz))
#define STBI_REALLOC(p,newsz)     (::aka::mem::getAllocator(::aka::AllocatorMemoryType::Object, ::aka::AllocatorCategory::Global).reallocate<StbAllocation>(p, newsz))
#define STBI_FREE(p)              (::aka::mem::getAllocator(::aka::AllocatorMemoryType::Object, ::aka::AllocatorCategory::Global).deallocate(p))

#include <stb_image.h>
#include <stb_image_write.h>

#include <stdexcept>

namespace aka {

template struct Img<byte_t>;
template struct Img<float>;

#if defined(AKA_ORIGIN_BOTTOM_LEFT)
constexpr const bool defaultFlipImageAtLoad = true;
constexpr const bool defaultFlipImageAtSave = true;
#else
constexpr const bool defaultFlipImageAtLoad = false;
constexpr const bool defaultFlipImageAtSave = false;
#endif

enum {
	STBI_rgb_DXT1 = 5, // BC1: 3 component
	STBI_rgb_DXT2 = 6, // BC2: 4 component
	STBI_rgb_DXT3 = 7, // BC2: 4 component
	STBI_rgb_DXT4 = 8, // BC3: 4 component
	STBI_rgb_DXT5 = 9, // BC3: 4 component
	STBI_rgb_BC4 = 10, // 1 component
	STBI_rgb_BC5 = 11, // 2 component
	STBI_bgr_alpha = 12,
	STBI_bgr = 13,
};
// https://learn.microsoft.com/fr-fr/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression
// https://fgiesen.wordpress.com/2021/10/04/gpu-bcn-decoding/
ImageComponent imageComponentFromStbiFormat(unsigned int stb_format)
{
	switch (stb_format) {
	case STBI_rgb:
		return ImageComponent::RGB;
	case STBI_rgb_alpha:
		return ImageComponent::RGBA;
	case STBI_grey:
		return ImageComponent::Red;
	case STBI_grey_alpha:
		return ImageComponent::Red | ImageComponent::Alpha;
	case STBI_rgb_DXT1:
		return ImageComponent::RGB;
	case STBI_rgb_DXT2:
		return ImageComponent::RGBA;
	case STBI_rgb_DXT3:
		return ImageComponent::RGBA;
	case STBI_rgb_DXT4:
		return ImageComponent::RGBA;
	case STBI_rgb_DXT5:
		return ImageComponent::RGBA;
	case STBI_bgr_alpha: // TODO: fix bgr
		return ImageComponent::RGBA;
	case STBI_bgr: // TODO: fix bgr
		return ImageComponent::RGB;
	case STBI_rgb_BC4:
		return ImageComponent::Red;
	case STBI_rgb_BC5:
		return ImageComponent::RG;
	default:
		AKA_NOT_IMPLEMENTED;
		return ImageComponent::None;
	}
}
uint32_t componentsFromStbiFormat(unsigned int stb_format)
{
	return getImageComponentCount(imageComponentFromStbiFormat(stb_format));
}
ImageCompression compressionFromStbiFormat(unsigned int stb_format)
{
	switch (stb_format) {
	case STBI_rgb:
	case STBI_rgb_alpha:
	case STBI_grey:
	case STBI_grey_alpha:
	case STBI_bgr_alpha:
	case STBI_bgr:
		return ImageCompression::None;
	case STBI_rgb_DXT1:
		return ImageCompression::Bc1;
	case STBI_rgb_DXT2:
		return ImageCompression::Bc2;
	case STBI_rgb_DXT3:
		return ImageCompression::Bc2;
	case STBI_rgb_DXT4:
		return ImageCompression::Bc3;
	case STBI_rgb_DXT5:
		return ImageCompression::Bc3;
	case STBI_rgb_BC4:
		return ImageCompression::Bc4;
	case STBI_rgb_BC5:
		return ImageCompression::Bc5;
	default:
		AKA_NOT_IMPLEMENTED;
		return ImageCompression::None;
	}
}
// Same signature as stb
// Return value is simply a pointer to blob content.
stbi_uc* loadDDS(const Blob& blob, int* x, int* y, int* stbi_format, int req_format)
{
	// https://github.com/daober/dds_loader/blob/master/src/dds_loader.cpp
#define DDSF_ALPHAPIXELS		0x00000001
#define DDSF_FOURCC				0x00000004
#define DDSF_RGB				0x00000040
#define DDSF_RGBA				0x00000041
	// https://docs.rs/ddsfile/latest/ddsfile/struct.FourCC.html
	enum FourCC {
		Dxt1 = 0x31545844, //(MAKEFOURCC('D','X','T','1'))
		Dxt2 = 0x32545844, //(MAKEFOURCC('D','X','T','2'))
		Dxt3 = 0x33545844, //(MAKEFOURCC('D','X','T','3'))
		Dxt4 = 0x34545844, //(MAKEFOURCC('D','X','T','4'))
		Dxt5 = 0x35545844, //(MAKEFOURCC('D','X','T','5'))
		Bc4Snorm = 0x53344342,
		Bc4Unorm = 0x55344342,
		Bc5Snorm = 0x53354342,
		Bc5Unorm = 0x32495441,
	};
	using DWORD = uint32_t;
	struct DDS_PIXELFORMAT {
		DWORD dwSize;
		DWORD dwFlags;
		DWORD dwFourCC;
		DWORD dwRGBBitCount;
		DWORD dwRBitMask;
		DWORD dwGBitMask;
		DWORD dwBBitMask;
		DWORD dwABitMask;
	};
	struct DDS_HEADER  {
		DWORD           dwSize;
		DWORD           dwFlags; 
		DWORD           dwHeight;
		DWORD           dwWidth;
		DWORD           dwPitchOrLinearSize;
		DWORD           dwDepth;
		DWORD           dwMipMapCount;
		DWORD           dwReserved1[11];
		DDS_PIXELFORMAT ddspf;
		DWORD           dwCaps;
		DWORD           dwCaps2;
		DWORD           dwCaps3;
		DWORD           dwCaps4;
		DWORD           dwReserved2;
	};
	const uint8_t* ptr = static_cast<const uint8_t*>(blob.data());
	if (*((const uint32_t*)ptr) != 0x20534444)
		return nullptr; // Invalid magic word
	ptr += 4;
	const DDS_HEADER* header = (const DDS_HEADER*)ptr;
	*x = (int)header->dwWidth;
	*y = (int)header->dwHeight;
	//if (header->dwCaps2 & DDSF_CUBEMAP)
	//else if (header->dwCaps2 & DDSF_VOLUME && header->dwDepth > 0)
	//else // Classic
	if (header->ddspf.dwFlags & DDSF_FOURCC)
	{
		switch (header->ddspf.dwFourCC) {
		case FourCC::Dxt1:
			*stbi_format = STBI_rgb_DXT1;
			break;
		case FourCC::Dxt2:
			*stbi_format = STBI_rgb_DXT2;
			break;
		case FourCC::Dxt3:
			*stbi_format = STBI_rgb_DXT3;
			break;
		case FourCC::Dxt4:
			*stbi_format = STBI_rgb_DXT4;
			break;
		case FourCC::Dxt5:
			*stbi_format = STBI_rgb_DXT5;
			break;
		case FourCC::Bc4Snorm: // TODO: support srgb
		case FourCC::Bc4Unorm:
			*stbi_format = STBI_rgb_BC4;
		case FourCC::Bc5Snorm: // TODO: support srgb
		case FourCC::Bc5Unorm:
			*stbi_format = STBI_rgb_BC5;
			break;
		default:
			// Unknown texture format.
			Logger::error("Unknown DDS fourcc : ", header->ddspf.dwFourCC);
			return nullptr;
		}
	}
	else if(header->ddspf.dwRGBBitCount == 32 &&
		header->ddspf.dwRBitMask == 0x00FF0000 &&
		header->ddspf.dwGBitMask == 0x0000FF00 &&
		header->ddspf.dwBBitMask == 0x000000FF &&
		header->ddspf.dwABitMask == 0xFF000000)
	{
		*stbi_format = STBI_bgr_alpha;
	}
	else if (header->ddspf.dwRGBBitCount == 32 &&
		header->ddspf.dwRBitMask == 0x000000FF &&
		header->ddspf.dwGBitMask == 0x0000FF00 &&
		header->ddspf.dwBBitMask == 0x00FF0000 &&
		header->ddspf.dwABitMask == 0xFF000000) 
	{
		*stbi_format = STBI_rgb_alpha;
	}
	else if (header->ddspf.dwRGBBitCount == 24 &&
		header->ddspf.dwRBitMask == 0x000000FF &&
		header->ddspf.dwGBitMask == 0x0000FF00 &&
		header->ddspf.dwBBitMask == 0x00FF0000) 
	{
		*stbi_format = STBI_rgb;
	}
	else if (header->ddspf.dwRGBBitCount == 24 &&
		header->ddspf.dwRBitMask == 0x00FF0000 &&
		header->ddspf.dwGBitMask == 0x0000FF00 &&
		header->ddspf.dwBBitMask == 0x000000FF) 
	{
		*stbi_format = STBI_bgr;
	}
	else if (header->ddspf.dwRGBBitCount == 8) {
		
		*stbi_format = STBI_grey; // LUMINANCE
	}
	else 
	{
		return nullptr; // Unknown format.
	}
	uint32_t numMipmap = header->dwMipMapCount ? header->dwMipMapCount : 0;

	DWORD width = *x;
	DWORD height = *y;
	DWORD channels = componentsFromStbiFormat(*stbi_format);
	AKA_ASSERT(req_format == *stbi_format || req_format == STBI_default, "Need to convert format.");
	ptr += sizeof(DDS_HEADER);
	// Compute size.
	DWORD pictureSize = getImageSize(width, height, imageComponentFromStbiFormat(*stbi_format), compressionFromStbiFormat(*stbi_format));
	AKA_ASSERT(pictureSize != 0, "Invalid size");
	AKA_ASSERT(pictureSize <= blob.size() - 4 - sizeof(DDS_HEADER), "Size too big");
	stbi_uc* data = (stbi_uc*)STBI_MALLOC(pictureSize);
	memcpy(data, ptr, pictureSize);
	return data;
}
stbi_uc* loadDDSFromFile(const Path& path, int* x, int* y, int* comp, int req_comp)
{
	Blob blob; 
	OS::File::read(path, &blob);
	return loadDDS(blob, x, y, comp, req_comp);
}
void freeDDS(stbi_uc* data)
{
	STBI_FREE(data);
}

uint32_t getImageComponentCount(ImageComponent _components)
{
	return countBitSet(toMask(_components));
}

size_t getImageSize(uint32_t width, uint32_t height, ImageComponent components, ImageCompression compression)
{
	// Bc has different bpe: https://github.com/microsoft/DirectXTK/blob/main/Src/LoaderHelpers.h#L498
	switch (compression)
	{
	case ImageCompression::None:
		return width * height * getImageComponentCount(components);
	case ImageCompression::Bc1:
	case ImageCompression::Bc4:
		return ((width + 3) / 4) * ((height + 3) / 4) * 8; // bpe = 8
	case ImageCompression::Bc2:
	case ImageCompression::Bc3:
	case ImageCompression::Bc5:
		return ((width + 3) / 4) * ((height + 3) / 4) * 16; // bpe = 16
		break;
	default:
		return 0;
	}
}

template <>
void Img<byte_t>::set(uint32_t x, uint32_t y, const Pixel& pixel)
{
	const uint32_t c = getImageComponentCount(components);
	size_t stride = c * width;
	memcpy(bytes.data() + x * c + stride * y, pixel.data, c);
}

template <>
void Img<float>::set(uint32_t x, uint32_t y, const Pixel& pixel)
{
	const uint32_t c = getImageComponentCount(components);
	size_t stride = sizeof(float) * c * width;
	memcpy(bytes.data() + x * c + stride * y, pixel.data, c * sizeof(float));
}
template <>
Img<byte_t>::Pixel Img<byte_t>::get(uint32_t x, uint32_t y) const
{
	Pixel out;
	const uint32_t c = getImageComponentCount(components);
	size_t stride = c * width;
	memcpy(out.data, bytes.data() + x * c + stride * y, c);
	return out;
}

template <>
Img<float>::Pixel Img<float>::get(uint32_t x, uint32_t y) const
{
	Pixel out;
	const uint32_t c = getImageComponentCount(components);
	size_t stride = sizeof(float) * c * width;
	memcpy(out.data, bytes.data() + x * c + stride * y, c * sizeof(float));
	return out;
}

ImageFileFormat ImageDecoder::getFileFormat(const Path& _path)
{
	return ImageFileFormat::Unknown;
}
ImageFileFormat ImageDecoder::getFileFormat(const Blob& _blob)
{
	return ImageFileFormat::Unknown;
}
Result<Image> ImageDecoder::fromDisk(const Path& _path)
{
	int x, y, stbi_format;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	stbi_uc* data = stbi_load(_path.cstr(), &x, &y, &stbi_format, STBI_default);
	// Try loading DDS
	if (data == nullptr && (data = loadDDSFromFile(_path, &x, &y, &stbi_format, STBI_default)) == nullptr)
	{
		return Result<Image>::error(Error{ String::format("Failed to decode image ", _path, " from disk: ", stbi_failure_reason()) });
	}
	else
	{
		Image img{};
		ImageComponent components = imageComponentFromStbiFormat(stbi_format);
		ImageCompression compression = compressionFromStbiFormat(stbi_format);
		size_t size = getImageSize(x, y, components, compression);
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size);
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = components;
		img.compression = compression;
		stbi_image_free(data);
		return Result<Image>::ok(img);
	}
}
Result<Image> ImageDecoder::fromMemory(const Blob& _blob)
{
	return fromMemory((const byte_t*)_blob.data(), _blob.size());
}
Result<Image> ImageDecoder::fromMemory(const byte_t* _data, size_t _size)
{
	int x, y, stbi_format;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	stbi_uc* data = stbi_load_from_memory(_data, (int)_size, &x, &y, &stbi_format, STBI_default);
	if (data == nullptr && (data = loadDDS(Blob(_data, _size), &x, &y, &stbi_format, STBI_default)) == nullptr)
	{
		return Result<Image>::error(Error{ String::format("Failed to decode image from memory: ", stbi_failure_reason()) });
	}
	else
	{
		Image img{};
		ImageComponent components = imageComponentFromStbiFormat(stbi_format);
		ImageCompression compression = compressionFromStbiFormat(stbi_format);
		size_t size = getImageSize(x, y, components, compression);
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size);
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = components;
		img.compression = compression;
		stbi_image_free(data);
		return Result<Image>::ok(img);
	}
}
Result<ImageHdr> ImageDecoder::fromDiskHdr(const Path& _path)
{
	int x, y, stbi_format;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	float* data = stbi_loadf(_path.cstr(), &x, &y, &stbi_format, STBI_default);
	if (data == nullptr)
	{
		return Result<ImageHdr>::error(Error{ String::format("Failed to decode hdr image ", _path, " from disk: ", stbi_failure_reason()) });
	}
	else
	{
		ImageHdr img{};
		ImageComponent components = imageComponentFromStbiFormat(stbi_format);
		ImageCompression compression = compressionFromStbiFormat(stbi_format);
		size_t size = getImageSize(x, y, components, compression);
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size * sizeof(float));
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = components;
		img.compression = compression;
		stbi_image_free(data);
		return Result<ImageHdr>::ok(img);
	}
}
Result<ImageHdr> ImageDecoder::fromMemoryHdr(const Blob& _blob)
{
	return fromMemoryHdr((const byte_t*)_blob.data(), _blob.size());
}
Result<ImageHdr> ImageDecoder::fromMemoryHdr(const byte_t* _data, size_t _size)
{
	int x, y, stbi_format;
	stbi_set_flip_vertically_on_load(defaultFlipImageAtLoad);
	float* data = stbi_loadf_from_memory(_data, (int)_size, &x, &y, &stbi_format, STBI_default);
	if (data == nullptr)
	{
		return Result<ImageHdr>::error(Error{ String::format("Failed to decode hdr image from memory: ", stbi_failure_reason()) });
	}
	else
	{
		ImageHdr img{};
		ImageComponent components = imageComponentFromStbiFormat(stbi_format);
		ImageCompression compression = compressionFromStbiFormat(stbi_format);
		size_t size = getImageSize(x, y, components, compression);
		img.bytes.resize(size);
		memcpy(img.bytes.data(), data, size * sizeof(float));
		img.width = static_cast<uint32_t>(x);
		img.height = static_cast<uint32_t>(y);
		img.components = components;
		img.compression = compression;
		stbi_image_free(data);
		return Result<ImageHdr>::ok(img);
	}
}

bool ImageEncoder::toDisk(const Path& _path, Image& _image, ImageFileFormat _format, ImageQuality _quality)
{
	const uint32_t c = getImageComponentCount(_image.components);
	AKA_ASSERT(_image.bytes.size() == _image.width * _image.height * c, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_jpg(_path.cstr(), _image.width, _image.height, c, _image.bytes.data(), (uint32_t)_quality * 50);
	return error != 0;
}
Result<Blob> ImageEncoder::toMemory(Image& _image, ImageFileFormat _format, ImageQuality _quality)
{
	AKA_NOT_IMPLEMENTED;
	/*const uint32_t c = getImageComponentCount(_image.components);
	AKA_ASSERT(_image.bytes.size() == _image.width * _image.height * c, "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	Blob data;
	int outLength = stbi_write_jpg_to_func([](void* context, void* data, int size) {
		Blob* bytes = reinterpret_cast<Blob*>(context);
		uint8_t* binaryData = static_cast<uint8_t*>(data);
		bytes->insert(bytes->end(), binaryData, binaryData + size);
		}, &data, _image.width, _image.height, c, _image.bytes.data(), (uint32_t)_quality * 50);
	if (data.size() == 0 || outLength == 0)
		return Blob();
	return data;*/
	return Result<Blob>::ok(Blob());
}
bool ImageEncoder::toDisk(const Path& _path, ImageHdr& _image, ImageFileFormat _format, ImageQuality _quality)
{
	/*AKA_ASSERT(m_format == ImageFormat::Float, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components * sizeof(float), "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	int error = stbi_write_hdr(path.cstr(), m_width, m_height, m_components, reinterpret_cast<const float*>(m_bytes.data()));
	return (error != 0);*/
	AKA_NOT_IMPLEMENTED;
	return false;
}
Result<Blob> ImageEncoder::toMemory(ImageHdr& _image, ImageFileFormat _format, ImageQuality _quality)
{
	/*AKA_ASSERT(m_format == ImageFormat::Float, "Invalid format");
	AKA_ASSERT(m_bytes.size() == m_width * m_height * m_components * sizeof(float), "Invalid size");
	stbi_flip_vertically_on_write(defaultFlipImageAtSave);
	std::vector<uint8_t> data;
	int outLength = stbi_write_hdr_to_func([](void* context, void* data, int size) {
		std::vector<uint8_t>* bytes = reinterpret_cast<std::vector<uint8_t>*>(context);
		uint8_t* binaryData = static_cast<uint8_t*>(data);
		bytes->insert(bytes->end(), binaryData, binaryData + size);
		}, &data, m_width, m_height, m_components, reinterpret_cast<const float*>(m_bytes.data()));
	if (data.size() == 0 || outLength == 0)
		return std::vector<uint8_t>();*/
	AKA_NOT_IMPLEMENTED;
	return Result<Blob>::ok(Blob());
}

};