#pragma once

#ifndef GRAPHICS_HEADER_FONT_H

#define GRAPHICS_HEADER_FONT_H

#include "common.h"
#include "texture.h"

typedef struct _FontType {
	float left;

	float right;
	
	int size;
} FontType;

typedef struct _FontVertexType {
	DirectX::XMFLOAT3 position;

	DirectX::XMFLOAT2 texture;
} FontVertexType;

class Font {
public:
	Font();

	Font(const Font& copy);

	~Font();

public:
	bool Initialize(
		char *font_filename,
		char *texture_filename,
		float font_height,
		int space_size
		);

	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	void BuildVertexArray(
		void *vertices, 
		char *sentence, 
		float draw_pos_x, 
		float draw_pos_y
		);

	int GetSentencePixelLength(char *sentence);

	int get_font_height();

private:
	bool LoadFontData(char *file_name);

	void ReleaseFontData();

	bool LoadTexture(char *texture_filename);

	void ReleaseTexture();

private:
	FontType *font_property_;
	
	Texture *texture_value_;

	float font_height_;

	int space_size_;
};

#endif // !GRAPHICS_HEADER_FONT_H
