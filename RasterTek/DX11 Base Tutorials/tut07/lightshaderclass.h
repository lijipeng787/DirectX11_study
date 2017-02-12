#ifndef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "d3dclass.h"

class LightShader{

private:

public:
	LightShader();

	explicit LightShader(const LightShader& copy);
	
	~LightShader();

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