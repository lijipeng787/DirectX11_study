

#pragma once

#include <d3d11.h>

class TextureClass {
public:
	TextureClass() {}

	TextureClass(const TextureClass&) = delete;

	~TextureClass() {}
public:
	bool Initialize(WCHAR*);

	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();
private:
	ID3D11ShaderResourceView * texture_ = nullptr;
};