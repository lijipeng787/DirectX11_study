#include "Texture.h"

#include "../../CommonFramework2/DirectX11Device.h"

#include <DDSTextureLoader.h>
#include <d3d11.h>
#include <stdexcept>
#include <vector>

using namespace DirectX;
using namespace std;

bool DDSTexture::Initialize(const WCHAR *filename, ID3D11Device *device) {
  auto result = CreateDDSTextureFromFile(device, filename, nullptr,
                                         texture_.GetAddressOf());
  if (FAILED(result)) {
    throw std::runtime_error("Failed to create texture from file");
  }
  return true;
}

bool TGATexture::Initialize(const char *filename, ID3D11Device *device) {

  D3D11_TEXTURE2D_DESC textureDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

  // Load the targa image data into memory.
  auto result = LoadTarga32Bit(filename);
  if (!result) {
    return false;
  }

  // Setup the description of the texture.
  textureDesc.Height = height_;
  textureDesc.Width = width_;
  textureDesc.MipLevels = 0;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.SampleDesc.Quality = 0;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

  // Create the empty texture.
  auto hResult =
      device->CreateTexture2D(&textureDesc, NULL, texture_.GetAddressOf());
  if (FAILED(hResult)) {
    return false;
  }

  // Set the row pitch of the targa image data.
  unsigned int rowPitch = (width_ * 4) * sizeof(unsigned char);

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Copy the targa image data into the texture.
  deviceContext->UpdateSubresource(texture_.Get(), 0, NULL, targa_data_,
                                   rowPitch, 0);

  // Setup the shader resource view description.
  srvDesc.Format = textureDesc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = -1;

  // Create the shader resource view for the texture.
  hResult = device->CreateShaderResourceView(texture_.Get(), &srvDesc,
                                             texture_view_.GetAddressOf());
  if (FAILED(hResult)) {
    return false;
  }

  // Generate mipmaps for this texture.
  deviceContext->GenerateMips(texture_view_.Get());

  // Release the targa image data now that the image data has been loaded into
  // the texture.
  delete[] targa_data_;
  targa_data_ = nullptr;

  return true;
}

bool TGATexture::LoadTarga32Bit(const char *filename) {
  int error, bpp, imageSize, index, i, j, k;
  FILE *filePtr;
  unsigned int count;
  TargaHeader targaFileHeader;

  // Open the targa file for reading in binary.
  error = fopen_s(&filePtr, filename, "rb");
  if (error != 0) {
    return false;
  }

  // Read in the file header.
  count =
      (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
  if (count != 1) {
    return false;
  }

  // Get the important information from the header.
  height_ = (int)targaFileHeader.height;
  width_ = (int)targaFileHeader.width;
  bpp = (int)targaFileHeader.bpp;

  // Check that it is 32 bit and not 24 bit.
  if (bpp != 32) {
    return false;
  }

  // Calculate the size of the 32 bit image data.
  imageSize = width_ * height_ * 4;

  std::vector<unsigned char> targaImage(imageSize);

  // Read in the targa image data.
  count = (unsigned int)fread(targaImage.data(), 1, imageSize, filePtr);
  if (count != imageSize) {
    return false;
  }

  // Close the file.
  error = fclose(filePtr);
  if (error != 0) {
    return false;
  }

  // Allocate memory for the targa destination data.
  targa_data_ = new unsigned char[imageSize];

  // Initialize the index into the targa destination data array.
  index = 0;

  // Initialize the index into the targa image data.
  k = (width_ * height_ * 4) - (width_ * 4);

  // Now copy the targa image data into the targa destination array in the
  // correct order since the targa format is stored upside down and also is not
  // in RGBA order.
  for (j = 0; j < height_; j++) {
    for (i = 0; i < width_; i++) {
      targa_data_[index + 0] = targaImage[k + 2]; // Red.
      targa_data_[index + 1] = targaImage[k + 1]; // Green.
      targa_data_[index + 2] = targaImage[k + 0]; // Blue
      targa_data_[index + 3] = targaImage[k + 3]; // Alpha

      // Increment the indexes into the targa data.
      k += 4;
      index += 4;
    }

    // Set the targa image data index back to the preceding row at the beginning
    // of the column since its reading it in upside down.
    k -= (width_ * 8);
  }

  return true;
}