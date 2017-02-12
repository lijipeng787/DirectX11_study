#pragma once

#ifndef GRAPHICS_HEADER_COLOR_SHADER_H

#define GRAPHICS_HEADER_COLOR_SHADER_H

#include "common.h"
#include "rendeable.h"

class ColorShader : public Renderable {
private:
	typedef struct _MatrixBufferType {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	} MatrixBufferType;

public:
	ColorShader();

	ColorShader(const ColorShader& copy);

	virtual ~ColorShader();

public:
	bool Initialize(HWND hwnd);
	
	bool InitializeGraphicsPipeline(
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX projection
		);

	virtual void DrawIndexed(const int index_count)override;

private:
	bool InitializeShadersAndInputLayout(HWND hwnd, WCHAR *vs_filename, WCHAR *ps_filename);

	bool InitializeConstantBuffers();

	bool SetGraphicsPipelineParameters(
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX projection
		);

	void SetShadersAndInputLayout();

private:
	int vertex_shader_ = -1;

	int pixel_shader_ = -1;

	int input_layout_ = -1;
	
	int matrix_buffer_ = -1;;
};

#endif // !GRAPHICS_HEADER_COLOR_SHADER_H
