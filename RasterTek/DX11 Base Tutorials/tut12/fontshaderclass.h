#ifndef _FONTSHADERCLASS_H_
#define _FONTSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "d3dclass.h"

class FontShader{

public:
	FontShader() {}
	
	explicit FontShader(const FontShader& copy) {}
	
	~FontShader() {}

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