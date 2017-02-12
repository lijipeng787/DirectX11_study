#pragma once

#ifndef GRAPHICS_HEADER_LIGHT_SHADER_H

#define GRAPHICS_HEADER_LIGHT_SHADER_H

#include "common.h"
#include "rendeable.h"

class LightShader :public Renderable {
private:
	typedef struct _MatrixBufferType {
		DirectX::XMMATRIX world;

		DirectX::XMMATRIX view;

		DirectX::XMMATRIX projection;
	} MatrixBufferType;

	typedef struct _LightBufferType {
		DirectX::XMFLOAT4 diffuser_color;

		DirectX::XMFLOAT3 light_direction;

		float padding;
	} LightBufferType;

public:
	LightShader();

	LightShader(const LightShader& copy);

	virtual ~LightShader();

public:
	bool Initialize(HWND hwnd);

	bool InitializeGraphicsPipeline(
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture,
		DirectX::XMFLOAT3 light_direction,
		DirectX::XMFLOAT4 light_diffuse_color
		);

	void DrawIndexed(const int index)override;

private:
	bool SetShadersAndInputLayout(HWND hwnd, WCHAR *vs_filename, WCHAR *ps_filename);

	bool SetGraphicsPipelineParameters(
		DirectX::XMMATRIX world, 
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture,
		DirectX::XMFLOAT3 light_direction, 
		DirectX::XMFLOAT4 light_diffuse_color
		);

	bool InitializeConstantBuffers();

	void SetShaderAndInputLayout();

	void SetSampler();

private:
	int vertex_shader_ = -1;

	int pixel_shader_ = -1;

	int input_layout_ = -1;

	int matrix_buffer_ = -1;

	int light_buffer_ = -1;

	int sampler_state_ = -1;
};

#endif // !GRAPHICS_HEADER_LIGHT_SHADER_H
