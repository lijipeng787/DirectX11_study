#pragma once

#ifndef APPLICATION_HEADER_TEXTURE_H
#define APPLICATION_HEADER_TEXTURE_H

#include <d3d11.h>
#include <stdio.h>
#include "d3d11device.h"

typedef struct _TargaHeader {
	unsigned char data1[12];

	unsigned short width;
	
	unsigned short height;
	
	unsigned char bpp;
	
	unsigned char data2;
} TargaHeader;

class Texture {
public:
	Texture();
	Texture(const Texture& copy);
	~Texture();

public:
	bool Initialize(char *file_name);

	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	// todo 1.add support for others texture format using DirectXTK toolkit
	bool LoadTarga(char *file_name, int& height, int& width);

private:
	unsigned char* taraga_data_;

	ID3D11Texture2D *texture_data_;

	ID3D11ShaderResourceView *texture_view_;
};

#endif // !APPLICATION_HEADER_TEXTURE_H
