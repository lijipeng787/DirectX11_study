#include "../CommonFramework/DirectX11Device.h"
#include "rendertextureclass.h"

using namespace DirectX;

bool RenderTextureClass::Initialize(int textureWidth, int textureHeight, float screenDepth, float screenNear) {

	texture_width_ = textureWidth;
	texture_height_ = textureHeight;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
	auto result = device->CreateTexture2D(&textureDesc, NULL, &render_target_texture_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(render_target_texture_, &renderTargetViewDesc, &render_target_view_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(render_target_texture_, &shaderResourceViewDesc, &shader_resource_view_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&depthBufferDesc, NULL, &depth_stencil_buffer_);
	if (FAILED(result)) {
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(depth_stencil_buffer_, &depthStencilViewDesc, &depth_stencil_view_);
	if (FAILED(result)) {
		return false;
	}

	viewport_.Width = (float)textureWidth;
	viewport_.Height = (float)textureHeight;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;

	projection_matrix_ = XMMatrixPerspectiveFovLH(((float)XM_PI / 4.0f), ((float)textureWidth / (float)textureHeight), screenNear, screenDepth);

	ortho_matrix_ = XMMatrixOrthographicLH((float)textureWidth, (float)textureHeight, screenNear, screenDepth);

	return true;
}

void RenderTextureClass::Shutdown() {
	if (depth_stencil_view_) {
		depth_stencil_view_->Release();
		depth_stencil_view_ = 0;
	}

	if (depth_stencil_buffer_) {
		depth_stencil_buffer_->Release();
		depth_stencil_buffer_ = 0;
	}

	if (shader_resource_view_) {
		shader_resource_view_->Release();
		shader_resource_view_ = 0;
	}

	if (render_target_view_) {
		render_target_view_->Release();
		render_target_view_ = 0;
	}

	if (render_target_texture_) {
		render_target_texture_->Release();
		render_target_texture_ = 0;
	}
}

void RenderTextureClass::SetRenderTarget() {

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->OMSetRenderTargets(1, &render_target_view_, depth_stencil_view_);

	device_context->RSSetViewports(1, &viewport_);
}

void RenderTextureClass::ClearRenderTarget(float red, float green, float blue, float alpha) {

	float color[4]{ red,green,blue,alpha };

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->ClearRenderTargetView(render_target_view_, color);

	device_context->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTextureClass::GetShaderResourceView() {
	return shader_resource_view_;
}

void RenderTextureClass::GetProjectionMatrix(XMMATRIX& projectionMatrix) {
	projectionMatrix = projection_matrix_;
}

void RenderTextureClass::GetOrthoMatrix(XMMATRIX& orthoMatrix) {
	orthoMatrix = ortho_matrix_;
}

int RenderTextureClass::GetTextureWidth() {
	return texture_width_;
}

int RenderTextureClass::GetTextureHeight() {
	return texture_height_;
}