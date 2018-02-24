#include <DDSTextureLoader.h>
#include "../CommonFramework/DirectX11Device.h"
#include "SimpleTexture.h"

void SimpleTexture::Release() {
	if (texture_srv_) {
		texture_srv_->Release();
		texture_srv_ = nullptr;
	}
}

bool SimpleTexture::LoadDDSTextureFromFile(WCHAR * ddsFilename) {

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	HRESULT result = CreateDDSTextureFromFile(device, ddsFilename, NULL, &texture_srv_);

	if (FAILED(result)) {
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* SimpleTexture::GetTexture(){
	return texture_srv_;
}