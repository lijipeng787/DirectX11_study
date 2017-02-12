#pragma once

#ifndef GRAPHICS_HEADER_SKYBOX_H

#define GRAPHICS_HEADER_SKYBOX_H

#include "common.h"

class SkyBox {
private:
	typedef struct _SkyBoxModelType {
		float x, y, z;

		float tu, tv;

		float nx, ny, nz;
	} SkyBoxModelType;

	typedef struct _VertexType {
		DirectX::XMFLOAT3 position;
	} VertexType;

public:
	SkyBox();

	SkyBox(const SkyBox& copy);

	~SkyBox();

public:
	bool Initialize();

	void Shutdown();

	void Render();

	int GetIndexCount();

	DirectX::XMFLOAT4 get_skybox_apex_color();

	DirectX::XMFLOAT4 get_skybox_center_color();

private:
	bool LoadSkyboxModel(char *model_filename);

	void ReleaseSkyboxModel();

	bool InitializeBuffers();

	void ReleaseBuffers();

	void RenderBuffers();

private:
	SkyBoxModelType *skybox_model_;

	int skybox_vertex_count_, skybox_index_count_;

	ID3D11Buffer *vertex_buffer_;

	ID3D11Buffer *index_buffer_;

	DirectX::XMFLOAT4 skybox_apex_color_, skybox_center_color_;
};

#endif // !GRAPHICS_HEADER_SKYBOX_H
