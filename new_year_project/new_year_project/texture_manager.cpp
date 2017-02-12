#include "texture_manager.h"

TextureManager::TextureManager() :texture_array_(nullptr), texture_count_(0) {
}

TextureManager::TextureManager(const TextureManager & copy){
}

TextureManager::~TextureManager(){
}

bool TextureManager::Initialize(int count){
	
	texture_count_ = count;

	texture_array_ = new Texture[texture_count_];
	if (!texture_array_)
		return false;

	return true;
}

void TextureManager::Shutdown(){
	if (texture_array_) {
		for (int i = 0; i < texture_count_; ++i) {
			texture_array_[i].Shutdown();
		}
		delete[] texture_array_;
		texture_array_ = nullptr;
	}
}

bool TextureManager::LoadTexture(char * file_name, int location) {
	if (!(texture_array_[location].Initialize(file_name))) {
		return false;
	}
	return true;
}

ID3D11ShaderResourceView * TextureManager::GetTexture(int id){
	return texture_array_[id].GetTexture();
}
