#ifndef _DEFERREDBUFFERSCLASS_H_
#define _DEFERREDBUFFERSCLASS_H_

const int BUFFER_COUNT = 2;

#include <DirectXMath.h>
#include <d3d11.h>

using namespace DirectX;

class DeferredBuffersClass {
public:
  DeferredBuffersClass();
  DeferredBuffersClass(const DeferredBuffersClass &);
  ~DeferredBuffersClass();

  bool Initialize(int, int, float, float);
  void Shutdown();

  void SetRenderTargets();
  void ClearRenderTargets(float, float, float, float);

  ID3D11ShaderResourceView *GetShaderResourceView(int);

private:
  int texture_width_, texture_height_;
  ID3D11Texture2D *m_renderTargetTextureArray[BUFFER_COUNT];
  ID3D11RenderTargetView *m_renderTargetViewArray[BUFFER_COUNT];
  ID3D11ShaderResourceView *m_shaderResourceViewArray[BUFFER_COUNT];
  ID3D11Texture2D *depth_stencil_buffer_;
  ID3D11DepthStencilView *depth_stencil_view_;
  D3D11_VIEWPORT viewport_;
};

#endif