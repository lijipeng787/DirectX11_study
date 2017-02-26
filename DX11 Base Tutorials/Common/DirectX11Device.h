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

	ID3D11Device* GetDevice();

	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(DirectX::XMMATRIX&);

	void GetWorldMatrix(DirectX::XMMATRIX&);

	void GetOrthoMatrix(DirectX::XMMATRIX&);

	void GetVideoCardInfo(char*, int&);
private:
	bool m_vsync_enabled = false;

	int m_videoCardMemory = 0;

	char m_videoCardDescription[128] = {};

	IDXGISwapChain* m_swapChain = nullptr;

	ID3D11Device* m_device = nullptr;

	ID3D11DeviceContext* m_deviceContext = nullptr;

	ID3D11RenderTargetView* m_renderTargetView = nullptr;

	ID3D11Texture2D* m_depthStencilBuffer = nullptr;

	ID3D11DepthStencilState* m_depthStencilState = nullptr;

	ID3D11DepthStencilView* m_depthStencilView = nullptr;

	ID3D11RasterizerState* m_rasterState = nullptr;

	DirectX::XMMATRIX m_projectionMatrix = {};

	DirectX::XMMATRIX m_worldMatrix = {};

	DirectX::XMMATRIX m_orthoMatrix = {};
};

#endif