
// Filename: orthowindowclass.h

#ifndef _ORTHOWINDOWCLASS_H_
#define _ORTHOWINDOWCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;



// Class name: OrthoWindowClass

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
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(int, int);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;
	int vertex_count_, index_count_;
};

#endif