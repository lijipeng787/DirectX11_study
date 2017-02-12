#pragma once

#ifndef GRAPHICS_HEADER_TERRAIN_SHADER_H

#define GRAPHICS_HEADER_TERRAIN_SHADER_H

#include "common.h"
#include "rendeable.h"

class TerrainShader :public Renderable {
public:
	typedef struct _MatrixBufferType {
		DirectX::XMMATRIX world;

		DirectX::XMMATRIX view;

		DirectX::XMMATRIX projection;
	} MatrixBufferType;

	typedef struct _LightBufferType {
		DirectX::XMFLOAT4 diffuse_color;

		DirectX::XMFLOAT3 light_direction;

		float padding;
	} LightBufferType;

public:
	TerrainShader();

	TerrainShader(const TerrainShader& copy);

	virtual ~TerrainShader();

public:
	bool Initialize(HWND hwnd);

	bool InitializeGraphicsPipeline(
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture,
		ID3D11ShaderResourceView *srv_normalmap,
		DirectX::XMFLOAT3 light_direction,
		DirectX::XMFLOAT4 diffuse_color
		);

	void DrawIndexed(const int index)override;

private:
	bool InitializeShadersAndInputLayout(HWND hwnd, WCHAR *vs_filename, WCHAR *ps_filename);

	bool InitalizeGraphicsPipelineParameters(
		DirectX::XMMATRIX world,
		DirectX::XMMATRIX view,
		DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture,
		ID3D11ShaderResourceView *srv_normalmap,
		DirectX::XMFLOAT3 light_direction,
		DirectX::XMFLOAT4 diffuse_color
		);

	bool InitializeConstantBuffers();

	bool InitializeSampler();

	void SetShadersAndInputLayout();

private:
	int vertex_shader_ = -1;

	int pixel_shader_ = -1;

	int input_layout_ = -1;

	int matrix_buffer_ = -1;

	int light_buffer_ = -1;

	int sampler_state_ = -1;
};

#endif // !GRAPHICS_HEADER_TERRAIN_SHADER_H
