#pragma once

#include <d3d11.h>

struct VertexType;

class DebugWindowClass {
public:
	DebugWindowClass() {}

	DebugWindowClass(const DebugWindowClass&) = delete;

	~DebugWindowClass() {}
public:
	bool Initialize(int, int, int, int);

	void Shutdown();

	bool Render(int, int);

	int GetIndexCount();
private:
	bool InitializeBuffers();

	void ShutdownBuffers();

	bool UpdateBuffers(int, int);

	void RenderBuffers();
private:
	ID3D11Buffer * vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

	int vertex_count_ = 0, index_count_ = 0;

	int m_screenWidth = 0, m_screenHeight = 0;

	int m_bitmapWidth = 0, m_bitmapHeight = 0;

	int m_previousPosX = 0, m_previousPosY = 0;
};
