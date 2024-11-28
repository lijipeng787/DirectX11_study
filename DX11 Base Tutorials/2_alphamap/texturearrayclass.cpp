#include <DDSTextureLoader.h>

#include "../CommonFramework/DirectX11Device.h"
#include "texturearrayclass.h"

using namespace DirectX;

bool TextureArrayClass::Initialize(WCHAR *filename1, WCHAR *filename2,
                                   WCHAR *filename3) {

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  auto result =
      CreateDDSTextureFromFile(device, filename1, NULL, &textures_[0]);
  if (FAILED(result)) {
    return false;
  }

  result = CreateDDSTextureFromFile(device, filename2, NULL, &textures_[1]);
  if (FAILED(result)) {
    return false;
  }

  // Load the third texture in.
  result = CreateDDSTextureFromFile(device, filename1, NULL, &textures_[2]);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void TextureArrayClass::Shutdown() {

  if (textures_[0]) {
    textures_[0]->Release();
    textures_[0] = nullptr;
  }

  if (textures_[1]) {
    textures_[1]->Release();
    textures_[1] = nullptr;
  }

  if (textures_[2]) {
    textures_[2]->Release();
    textures_[2] = 0;
  }
}

ID3D11ShaderResourceView **TextureArrayClass::GetTextureArray() {
  return textures_;
}
