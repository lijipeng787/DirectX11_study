#include "texture.h"

Texture::Texture() :taraga_data_(nullptr), texture_data_(nullptr), texture_view_(nullptr) {
}

bool Texture::Initialize(char * file_name){
	
	auto device = D3d11Device::get_d3d_object()->get_device();
	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	int height = 0;
	int width = 0;
	if (!(LoadTarga(file_name, height, width))) {
		return false;
	}

	D3D11_TEXTURE2D_DESC texture_desc;
	ZeroMemory(&texture_desc, sizeof(texture_desc));

	texture_desc.Height = height;
	texture_desc.Width = width;
	texture_desc.MipLevels = 0;
	texture_desc.ArraySize = 1;
	texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture_desc.CPUAccessFlags = 0;
	texture_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	if (FAILED(device->CreateTexture2D(&texture_desc, nullptr, &texture_data_))) {
		return false;
	}

	auto row_pitch = static_cast<UINT>((width * 4)*sizeof(unsigned char));

	device_context->UpdateSubresource(texture_data_, 0, nullptr, taraga_data_, row_pitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	ZeroMemory(&shader_resource_view_desc, sizeof(shader_resource_view_desc));

	shader_resource_view_desc.Format = texture_desc.Format;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
	shader_resource_view_desc.Texture2D.MipLevels = -1;

	if (FAILED(device->CreateShaderResourceView(texture_data_, &shader_resource_view_desc, &texture_view_))) {
		return false;
	}

	device_context->GenerateMips(texture_view_);

	delete[] taraga_data_;
	taraga_data_ = nullptr;

	return true;
}

void Texture::Shutdown(){

	if (texture_view_) {
		texture_view_->Release();
		texture_view_ = nullptr;
	}

	if (texture_data_) {
		texture_data_->Release();
		texture_data_ = nullptr;
	}

	if (taraga_data_) {
		delete[] taraga_data_;
		taraga_data_ = nullptr;
	}
}

ID3D11ShaderResourceView * Texture::GetTexture(){
	return texture_view_;
}

bool Texture::LoadTarga(char * file_name, int & height, int & width){

	FILE *file;
	auto return_status = fopen_s(&file, file_name, "rb");
	if (0 != return_status)
		return false;

	TargaHeader targa_file_header;
	ZeroMemory(&targa_file_header, sizeof(targa_file_header));

	auto count = fread(&targa_file_header, sizeof(targa_file_header), 1, file);
	if (1 != count)
		return false;

	height = targa_file_header.height;
	width = targa_file_header.width;
	auto bpp = targa_file_header.bpp;

	if (32 != bpp)
		return false;

	auto image_size = width*height * 4;

	auto targa_image = new unsigned char[image_size];
	if (!targa_image)
		return false;

	count = fread(targa_image, 1, image_size, file);
	if (image_size != count)
		return false;

	return_status = fclose(file);
	if (0 != return_status)
		return false;

	this->taraga_data_ = new unsigned char[image_size];
	if (!taraga_data_)
		return false;

	auto dest_index = 0;
	auto src_index = (width*height * 4) - (width * 4);

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			taraga_data_[dest_index + 0] = targa_image[src_index + 2];
			taraga_data_[dest_index + 1] = targa_image[src_index + 1];
			taraga_data_[dest_index + 2] = targa_image[src_index + 0];
			taraga_data_[dest_index + 3] = targa_image[src_index + 3];
			src_index += 4;
			dest_index += 4;
		}
		src_index -= (width * 8);
	}
	
	delete[] targa_image;
	targa_image = nullptr;

	return true;
}

Texture::Texture(const Texture & copy){
}

Texture::~Texture(){
}
