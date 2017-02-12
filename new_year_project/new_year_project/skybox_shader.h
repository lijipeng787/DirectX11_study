#pragma once

#ifndef GRAPHICS_HEADER_SKYBOX_SHADER_H

#define GRAPHICS_HEADER_SKYBOX_SHADER_H

#include "common.h"
#include "rendeable.h"

class SkyboxShader :public Renderable {
private:
	typedef struct _MatrixBufferType {
		DirectX::XMMATRIX world;

		DirectX::XMMATRIX view;

		DirectX::XMMATRIX projection;
	} MatrixBufferType;

	typedef struct SkyboxColorBufferType {
		DirectX::XMFLOAT4 apex_color;

		DirectX::XMFLOAT4 center_color;
	} SkyboxColorBufferType;

public:
	SkyboxShader();

	SkyboxShader(const SkyboxShader& copy);

	virtual ~SkyboxShader();

public:
	bool Initialize(HWND hwnd);

	bool InitializeGraphicsPipeline(
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX projection,
		DirectX::XMFLOAT4 apex_color, 
		DirectX::XMFLOAT4 center_color
		);

	void DrawIndexed(const int index)override;

private:
	bool InitializeShadersAndInputLayout(HWND hwnd, WCHAR *vs_filename, WCHAR *ps_filename);

	bool InitializeGraphicsPipelineParameters(
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view, 
		DirectX::XMMATRIX projection,
		DirectX::XMFLOAT4 apex_color,
		DirectX::XMFLOAT4 center_color
		);

	bool InitializeConstantBuffers();

	void SetShadersAndInputLayout();

private:
	int vertex_shader_ = -1;

	int pixel_shader_ = -1;

	int input_layout_ = -1;

	int matrix_buffer_ = -1;

	int skybox_color_buffer_ = -1;
};

#endif // !GRAPHICS_HEADER_SKYBOX_SHADER_H
