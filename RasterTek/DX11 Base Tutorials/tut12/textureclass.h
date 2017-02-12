#ifndef HEADER_TEXTURECLASS_H
#define HEADER_TEXTURECLASS_H

#include "d3dclass.h"
#include "DDSTextureLoader.h"

class Texture
{
public:
	Texture() {}

	explicit Texture(const Texture& copy) {}

	~Texture() {}

public:
	bool set_texture(WCHAR *texture_filename);

	DescriptorHeapPtr get_texture();

private:
	D3d12ResourcePtr texture_ = nullptr;

	DescriptorHeapPtr shader_resource_view_heap_ = nullptr;
};


#endif