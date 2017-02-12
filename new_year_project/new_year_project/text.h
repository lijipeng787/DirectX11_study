#pragma once

#ifndef GRAPHICS_HEADER_TEXT_H

#define GRAPHICS_HEADER_TEXT_H

#include "font.h"
#include "shader_manager.h"
#include "rendeable.h"

class Text :public Renderable {
private:
	struct VertexType {
		DirectX::XMFLOAT3 position;

		DirectX::XMFLOAT2 texture;
	};

public:
	Text();

	Text(const Text &copy);

	~Text();

public:
	bool Initialize(
		int screen_width, int screen_height,
		int max_length, bool shadow,
		Font *font_object,
		char *text,
		int position_x, int position_y, 
		float red, float green, float blue
		);

	void Shutdown();

	void Render(
		ShaderManager *shader_manager_object,
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX orthogonality,
		ID3D11ShaderResourceView *srv_font_texture
		);

	bool UpdateSentence(
		Font *font_object,
		char *text,
		int position_x, int position_y, 
		float red, float green, float blue
		);

private:
	bool InitializeSentence(
		Font *font_object,
		char *text, 
		int position_x, int position_y, 
		float red, float green, float blue
		);

	void RenderSentence(
		ShaderManager *shader_manager_object,
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX orthogonality,
		ID3D11ShaderResourceView *srv_font_texture
		);

private:
	ID3D11Buffer *vertex_buffer_, *shadow_vertex_buffer_;

	ID3D11Buffer *index_buffer_, *shadow_index_buffer_;

	int screen_width_, scrren_height_, max_length_, vertex_count_, index_count_;

	bool shadow_;

	DirectX::XMFLOAT4 pixel_color_;
};

#endif // !GRAPHICS_HEADER_TEXT_H
