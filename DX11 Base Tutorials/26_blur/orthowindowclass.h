#pragma once

#include <d3d11.h>

struct VertexType;

class OrthoWindowClass {
public:
	OrthoWindowClass() {}

	OrthoWindowClass(const OrthoWindowClass&) = delete;

	~OrthoWindowClass() {}
public:
	bool Initialize(int, int);

	void Shutdown();

	void Render();

	int GetIndexCount();
private:
	bool InitializeBuffers(int, int);

	void ShutdownBuffers();

	void RenderBuffers();
private:
	ID3D11Buffer * vertex_buffer_ = nullptr, *index_buffer_ = nullptr;
	int vertex_count_ = 0, index_count_ = 0;
};
