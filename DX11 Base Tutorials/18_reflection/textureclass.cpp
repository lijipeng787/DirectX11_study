#include <DDSTextureLoader.h>

#include "../CommonFramework/DirectX11Device.h"
#include "textureclass.h"

using namespace DirectX;

bool TextureClass::Initialize(WCHAR *filename) {

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  auto result = CreateDDSTextureFromFile(device, filename, NULL, &texture_);
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

ID3D11ShaderResourceView *TextureClass::GetTexture() { return texture_; }