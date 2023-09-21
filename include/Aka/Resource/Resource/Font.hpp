#pragma once

#include <Aka/Resource/Resource/Resource.hpp>

namespace aka {
// .font->hold the ttf to be parsed or other depending on method(check valve paper with sdf).
//
struct ArchiveFont : Archive {
	Blob ttf;
	void load(const Path& path) override;
	void save(const Path& path) override;
};
struct Font : Resource {
public:
	void create_internal(const ArchiveFont& _archive);
	void destroy_internal(const ArchiveFont& _archive);

};

}