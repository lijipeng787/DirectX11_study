#pragma once

#include <d3d11.h>

using namespace DirectX;

struct VertexType;

class ModelClass {
public:
	ModelClass() {}

	ModelClass(const ModelClass&) = delete;

	~ModelClass() {}
public:
	bool Initialize();

	void Shutdown();

	void Render();

	int GetIndexCount();
private:
	bool InitializeBuffers();

	void ShutdownBuffers();

	void RenderBuffers();
private:
	ID3D11Buffer* vertex_buffer_ = nullptr, * index_buffer_ = nullptr;

	int vertex_count_ = 0, index_count_ = 0;
};
