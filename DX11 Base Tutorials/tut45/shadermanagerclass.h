
// Filename: shadermanagerclass.h

#ifndef _SHADERMANAGERCLASS_H_
#define _SHADERMANAGERCLASS_H_





#include "textureshaderclass.h"
#include "lightshaderclass.h"
#include "bumpmapshaderclass.h"



// Class name: ShaderManagerClass

class ShaderManagerClass
{
public:
	ShaderManagerClass();
	ShaderManagerClass(const ShaderManagerClass&);
	~ShaderManagerClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();

	bool RenderTextureShader(int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*);

	bool RenderLightShader(int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, 
						   const XMFLOAT3&, const XMFLOAT4&, const XMFLOAT4&,const XMFLOAT3&, const XMFLOAT4&, float );

	bool RenderBumpMapShader(int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, 
							 ID3D11ShaderResourceView*, const XMFLOAT3&, const XMFLOAT4& );

private:
	TextureShaderClass* m_TextureShader;
	LightShaderClass* light_shader_;
	BumpMapShaderClass* m_BumpMapShader;
};

#endif