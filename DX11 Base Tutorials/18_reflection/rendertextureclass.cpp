#include "../CommonFramework/DirectX11Device.h"
#include "rendertextureclass.h"

bool RenderTextureClass::Initialize(int textureWidth, int textureHeight) {

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

	return true;
}

void RenderTextureClass::Shutdown() {
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

void RenderTextureClass::SetRenderTarget(ID3D11DepthStencilView* depthStencilView) {

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
	device_context->OMSetRenderTargets(1, &render_target_view_, depthStencilView);
}

void RenderTextureClass::ClearRenderTarget(ID3D11DepthStencilView* depthStencilView,
										   float red, float green, float blue, float alpha) {
	float color[4] = { red,green,blue,alpha };
	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->ClearRenderTargetView(render_target_view_, color);

	device_context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTextureClass::GetShaderResourceView() {
	return shader_resource_view_;
}