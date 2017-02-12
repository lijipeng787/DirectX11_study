#pragma once

#ifndef APPLICATION_HEADER_TEXTURE_MANAGER_H
#define APPLICATION_HEADER_TEXTURE_MANAGER_H

#include "texture.h"

class TextureManager {
public:
	TextureManager();
	
	TextureManager(const TextureManager& copy);
	
	~TextureManager();

public:
	bool Initialize(int count);

	void Shutdown();

	bool LoadTexture(char *file_name, int location);

	ID3D11ShaderResourceView* GetTexture(int id);

private:
	Texture *texture_array_;

	int texture_count_;
};

#endif // !APPLICATION_HEADER_TEXTURE_MANAGER_H
