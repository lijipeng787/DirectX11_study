#include "textureclass.h"

#include "../../DirectXTK/Inc/DDSTextureLoader.h"

#include "../CommonFramework/DirectX11Device.h"

using namespace DirectX;

TextureClass::TextureClass() {
	texture_ = nullptr;
}


TextureClass::TextureClass(const TextureClass& other) {
}


TextureClass::~TextureClass() {
}


bool TextureClass::Initialize(WCHAR* filename) {
	HRESULT result;

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	result = CreateDDSTextureFromFile(device, filename, nullptr, &texture_);
	if (FAILED(result)) {
		return false;
	}

	return true;
}


void TextureClass::Shutdown() {

	if (texture_) {
		texture_->Release();
		texture_ = nullptr;
	}


}


ID3D11ShaderResourceView* TextureClass::GetTexture() {
	return texture_;
}