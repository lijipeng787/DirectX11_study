#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_

#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dclass.h"

class TextureShader
{
public:
	TextureShader();

	explicit TextureShader(const TextureShader& copy);
	
	~TextureShader();
		
public:
	bool Initialize(WCHAR *vs_shader_filename, WCHAR *ps_shader_filename);

	BlobPtr GetVertexShaderBlob()const;

	BlobPtr GetPixelSHaderBlob()const;

private:
	BlobPtr vertex_shader_blob_ = nullptr;

	BlobPtr pixel_shader_blob_ = nullptr;

	BlobPtr shader_error_blob = nullptr;
};

#endif