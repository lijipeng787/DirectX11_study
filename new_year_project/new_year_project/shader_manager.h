#pragma once

#ifndef GRAPHICS_HEADER_SHADER_MANAGER_H

#define GRAPHICS_HEADER_SHADER_MANAGER_H

#include "d3d11device.h"
#include "color_shader.h"
#include "texture_shader.h"
#include "light_shader.h"
#include "font_shader.h"
#include "skybox_shader.h"
#include "terrain_shader.h"

class ShaderManager {
public:
	ShaderManager();

	ShaderManager(const ShaderManager& copy);

	~ShaderManager();

public:
	bool Initialize(HWND hwnd);

	bool ExecuteColorShader(int index_count, DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection);

	bool ExecuteTextureShader(int index_count,
		DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture);

	bool ExecuteLightShader(int index_count,
		DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView * srv_texture,
		DirectX::XMFLOAT3 light_direction, DirectX::XMFLOAT4 diffuse_color);

	bool ExecuteFontShader(int index_count,
		DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture, DirectX::XMFLOAT4 font_color);

	bool ExecuteSkyboxShader(int index_count,
		DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
		DirectX::XMFLOAT4 skybox_apex_color, DirectX::XMFLOAT4 skybox_center_color);

	bool ExecuteTerrainShader(int index_count,
		DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection,
		ID3D11ShaderResourceView *srv_texture, ID3D11ShaderResourceView *srv_normalmap,
		DirectX::XMFLOAT3 light_direction, DirectX::XMFLOAT4 diffuse_color);

private:
	ColorShader *color_shader_ = nullptr;

	TextureShader *texture_shader_ = nullptr;

	LightShader *light_shader_ = nullptr;

	FontShader *font_shader_ = nullptr;

	SkyboxShader *skybox_shader_ = nullptr;

	TerrainShader *terrain_shader_ = nullptr;
};

#endif // !GRAPHICS_HEADER_SHADER_MANAGER_H
