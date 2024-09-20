#ifndef _ORTHOWINDOWCLASS_H_
#define _ORTHOWINDOWCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class OrthoWindowClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	OrthoWindowClass();
	OrthoWindowClass(const OrthoWindowClass&);
	~OrthoWindowClass();

	bool Initialize(int, int);
	void Shutdown();
	void Render();

	int GetIndexCount();

private:
	bool InitializeBuffers(int, int);
	void ShutdownBuffers();
	void RenderBuffers();

private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;
	int vertex_count_, index_count_;
};

#endif