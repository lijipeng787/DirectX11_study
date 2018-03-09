
// Filename: rendertextureclass.h

#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_





#include <d3d11.h>



// Class name: RenderTextureClass

class RenderTextureClass
{
public:
	RenderTextureClass();
	RenderTextureClass(const RenderTextureClass&);
	~RenderTextureClass();

	bool Initialize(int, int);
	void Shutdown();

	void SetRenderTarget(ID3D11DepthStencilView*);
	void ClearRenderTarget(ID3D11DepthStencilView*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();

private:
	ID3D11Texture2D* render_target_texture_;
	ID3D11RenderTargetView* render_target_view_;
	ID3D11ShaderResourceView* shader_resource_view_;
};

#endif