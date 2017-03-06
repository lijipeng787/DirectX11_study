#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

class DirectX11Device {
public:
	DirectX11Device();

	DirectX11Device(const DirectX11Device& rhs) = delete;

	DirectX11Device& operator=(const DirectX11Device& rhs) = delete;

	~DirectX11Device();
public:
	bool Initialize(int, int, bool, HWND, bool, float, float);

	void Shutdown();

	void BeginScene(float, float, float, float);

	void EndScene();

	void TurnZBufferOn();

	void TurnZBufferOff();

	void TurnOnAlphaBlending();

	void TurnOffAlphaBlending();

	void TurnOnCulling();
		 
	void TurnOffCulling();

	void SetBackBufferRenderTarget();

	void ResetViewport();

	ID3D11Device* GetDevice();

	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(DirectX::XMMATRIX& projection_matrix);

	void GetWorldMatrix(DirectX::XMMATRIX& world_matrix);

	void GetOrthoMatrix(DirectX::XMMATRIX& orthonality_matrix);

	void GetVideoCardInfo(char*, int&);

	ID3D11DepthStencilView* GetDepthStencilView()const;
private:
	bool vsync_enabled_ = false;

	unsigned int videocard_Memory_ = 0;

	char videocard_description_[128] = {};

	IDXGISwapChain* swap_chain_ = nullptr;

	ID3D11Device* device_ = nullptr;

	ID3D11DeviceContext* device_context_ = nullptr;

	ID3D11RenderTargetView* render_target_view_ = nullptr;

	ID3D11Texture2D* depth_stencil_buffer_ = nullptr;

	ID3D11DepthStencilState* depth_stencil_state_ = nullptr;

	ID3D11DepthStencilView* depth_stencil_view_ = nullptr;

	ID3D11RasterizerState* raster_state_ = nullptr;

	ID3D11RasterizerState* m_rasterStateNoCulling = nullptr;

	DirectX::XMMATRIX projection_matrix_ = {};

	DirectX::XMMATRIX world_matrix_ = {};

	DirectX::XMMATRIX orthonality_matrix_ = {};

	ID3D11DepthStencilState* m_depthDisabledStencilState;

	ID3D11BlendState* m_alphaEnableBlendingState;

	ID3D11BlendState* m_alphaDisableBlendingState;

	D3D11_VIEWPORT viewport_ = {};
};

#endif