#pragma once

#include <d3d11.h>

using namespace DirectX;

class SimpleTexture {
public:
	SimpleTexture() {}

	SimpleTexture(const SimpleTexture& rhs) = delete;

	SimpleTexture& operator=(const SimpleTexture& rhs) = delete;

	~SimpleTexture() {}
public:
	bool LoadDDSTextureFromFile(WCHAR *ddsFilename);

	ID3D11ShaderResourceView* GetTexture();

	void Release();
private:
	ID3D11ShaderResourceView * texture_srv_;
};
