
// Filename: textureclass.h

#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_





#include <d3d11.h>
#include <DDSTextureLoader.h>
using namespace DirectX;



// Class name: TextureClass

class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11ShaderResourceView* texture_;
};

#endif