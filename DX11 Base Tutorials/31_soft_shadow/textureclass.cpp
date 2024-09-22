//
//
// #include "textureclass.h"
// #include "../CommonFramework/DirectX11Device.h"
//
// TextureClass::TextureClass() { texture_ = nullptr; }
//
// TextureClass::TextureClass(const TextureClass &other) {}
//
// TextureClass::~TextureClass() {}
//
// bool TextureClass::Initialize(WCHAR *filename) {
//  HRESULT result;
//
//  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
//
//  result = CreateDDSTextureFromFile(device, filename, NULL, &texture_);
//  if (FAILED(result)) {
//    return false;
//  }
//
//  return true;
//}
//
// void TextureClass::Shutdown() {
//
//  if (texture_) {
//    texture_->Release();
//    texture_ = nullptr;
//  }
//}
//
// ID3D11ShaderResourceView *TextureClass::GetTexture() { return texture_; }

#include "TextureClass.h"

#include "../CommonFramework/DirectX11Device.h"

#include <DDSTextureLoader.h>
#include <d3d11.h>
#include <stdexcept>

bool TextureClass::Initialize(const WCHAR *filename) {
  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  auto result = DirectX::CreateDDSTextureFromFile(device, filename, nullptr,
                                                  texture_.GetAddressOf());
  if (FAILED(result)) {
    throw std::runtime_error("Failed to create texture from file");
  }
  return true;
}

void TextureClass::Shutdown() { texture_.Reset(); }

ID3D11ShaderResourceView *TextureClass::GetTexture() const {
  return texture_.Get();
}