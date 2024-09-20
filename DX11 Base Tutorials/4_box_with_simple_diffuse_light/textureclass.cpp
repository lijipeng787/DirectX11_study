#include "TextureClass.h"

#include "../CommonFramework/DirectX11Device.h"

#include <d3d11.h>
#include <DDSTextureLoader.h>
#include <stdexcept>

bool TextureClass::Initialize(const WCHAR* filename) {
    auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
    auto result = DirectX::CreateDDSTextureFromFile(device, filename, nullptr, texture_.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create texture from file");
    }
    return true;
}

void TextureClass::Shutdown() {
    texture_.Reset();
}

ID3D11ShaderResourceView* TextureClass::GetTexture() const {
    return texture_.Get();
}