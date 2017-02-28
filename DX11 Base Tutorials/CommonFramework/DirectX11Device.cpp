#include "stdafx.h"
#include "DirectX11Device.h"

using namespace DirectX;

DirectX11Device::DirectX11Device() {}

DirectX11Device::~DirectX11Device(){}

bool DirectX11Device::Initialize(
	int screenWidth, int screenHeight,
	bool vsync, HWND hwnd, bool fullscreen,
	float screenDepth, float screenNear
) {

	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	size_t stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	vsync_enabled_ = vsync;

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)) {
		return false;
	}

	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result)) {
		return false;
	}

	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result)) {
		return false;
	}

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)) {
		return false;
	}

	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList) {
		return false;
	}

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)) {
		return false;
	}

	for (i = 0; i < numModes; i++) {

		if (displayModeList[i].Width == (unsigned int)screenWidth) {

			if (displayModeList[i].Height == (unsigned int)screenHeight) {
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)) {
		return false;
	}

	videocard_Memory_ = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	error = wcstombs_s(&stringLength, videocard_description_, 128, adapterDesc.Description, 128);
	if (error != 0) {
		return false;
	}

	delete[] displayModeList;
	displayModeList = 0;

	adapterOutput->Release();
	adapterOutput = 0;

	adapter->Release();
	adapter = 0;

	factory->Release();
	factory = 0;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (vsync_enabled_) {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	if (fullscreen) {
		swapChainDesc.Windowed = false;
	}
	else {
		swapChainDesc.Windowed = true;
	}

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;

	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &swap_chain_, &device_, NULL, &device_context_);
	if (FAILED(result)) {
		return false;
	}

	result = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) {
		return false;
	}

	result = device_->CreateRenderTargetView(backBufferPtr, NULL, &render_target_view_);
	if (FAILED(result)) {
		return false;
	}

	backBufferPtr->Release();
	backBufferPtr = 0;

	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	result = device_->CreateTexture2D(&depthBufferDesc, NULL, &depth_stencil_buffer_);
	if (FAILED(result)) {
		return false;
	}

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = device_->CreateDepthStencilState(&depthStencilDesc, &depth_stencil_state_);
	if (FAILED(result)) {
		return false;
	}

	device_context_->OMSetDepthStencilState(depth_stencil_state_, 1);

	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device_->CreateDepthStencilView(depth_stencil_buffer_, &depthStencilViewDesc, &depth_stencil_view_);
	if (FAILED(result)) {
		return false;
	}

	device_context_->OMSetRenderTargets(1, &render_target_view_, depth_stencil_view_);

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = device_->CreateRasterizerState(&rasterDesc, &raster_state_);
	if (FAILED(result)) {
		return false;
	}

	device_context_->RSSetState(raster_state_);

	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	device_context_->RSSetViewports(1, &viewport);

	fieldOfView = (float)XM_PI / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

	projection_matrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	world_matrix_ = XMMatrixIdentity();

	orthonality_matrix_ = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}

void DirectX11Device::Shutdown() {

	if (swap_chain_) {
		swap_chain_->SetFullscreenState(false, NULL);
	}

	if (raster_state_) {
		raster_state_->Release();
		raster_state_ = 0;
	}

	if (depth_stencil_view_) {
		depth_stencil_view_->Release();
		depth_stencil_view_ = 0;
	}

	if (depth_stencil_state_) {
		depth_stencil_state_->Release();
		depth_stencil_state_ = 0;
	}

	if (depth_stencil_buffer_) {
		depth_stencil_buffer_->Release();
		depth_stencil_buffer_ = 0;
	}

	if (render_target_view_) {
		render_target_view_->Release();
		render_target_view_ = 0;
	}

	if (device_context_) {
		device_context_->Release();
		device_context_ = 0;
	}

	if (device_) {
		device_->Release();
		device_ = 0;
	}

	if (swap_chain_) {
		swap_chain_->Release();
		swap_chain_ = 0;
	}
}

void DirectX11Device::BeginScene(float red, float green, float blue, float alpha) {

	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	device_context_->ClearRenderTargetView(render_target_view_, color);

	device_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DirectX11Device::EndScene() {

	if (vsync_enabled_) {
		swap_chain_->Present(1, 0);
	}
	else {
		swap_chain_->Present(0, 0);
	}
}

ID3D11Device* DirectX11Device::GetDevice(){
	return device_;
}

ID3D11DeviceContext* DirectX11Device::GetDeviceContext(){
	return device_context_;
}

void DirectX11Device::GetProjectionMatrix(XMMATRIX& projectionMatrix) {
	projectionMatrix = projection_matrix_;
}

void DirectX11Device::GetWorldMatrix(XMMATRIX& worldMatrix) {
	worldMatrix = world_matrix_;
}

void DirectX11Device::GetOrthoMatrix(XMMATRIX& orthoMatrix) {
	orthoMatrix = orthonality_matrix_;
}

void DirectX11Device::GetVideoCardInfo(char* cardName, int& memory) {
	strcpy_s(cardName, 128, videocard_description_);
	memory = videocard_Memory_;
}
