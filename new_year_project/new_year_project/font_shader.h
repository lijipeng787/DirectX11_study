#pragma once

#ifndef GRAPHICS_HEADER_FONT_SHADER_H

#define GRAPHICS_HEADER_FONT_SHADER_H

#include "common.h"
#include "rendeable.h"

class FontShader :public Renderable{
private:
	typedef struct _MatrixBufferType {
		DirectX::XMMATRIX world;

		DirectX::XMMATRIX view;

		DirectX::XMMATRIX projection;
	} MatrixBufferType;

	typedef struct _PixelColorBufferType {
		DirectX::XMFLOAT4 pixel_color;
	} PixelColorBufferType;

public:
	FontShader();

	FontShader(const FontShader& copy);

	virtual ~FontShader();

public:
	bool Initialize(HWND hwnd);

	bool InitializeGraphicsPipeline(
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture, 
		DirectX::XMFLOAT4 pixel_color
		);

	virtual void DrawIndexed(const int index_count)override;

private:
	bool InitializeShadersAndInputLayout(HWND hwnd, WCHAR *vs_filename, WCHAR *ps_filename);

	bool SetGraphicsPipelineParameters(
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture,
		DirectX::XMFLOAT4 pixel_color
		);

	void SetShaderAndInputLayout();

	void SetSamplerState();

	bool InitializeConstantBuffers();

private:
	int vertex_shader_ = -1;
	
	int pixel_shader_ = -1;
	
	int input_layout_ = -1;
	
	int matrix_buffer_ = -1;
	
	int pixel_color_buffer_ = -1;
	
	int sampler_state_ = -1;
};

#endif // !GRAPHICS_HEADER_FONT_SHADER_H
