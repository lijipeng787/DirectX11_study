
// Filename: debugwindowclass.h

#ifndef _DEBUGWINDOWCLASS_H_
#define _DEBUGWINDOWCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;



// Class name: DebugWindowClass

class DebugWindowClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	DebugWindowClass();
	DebugWindowClass(const DebugWindowClass&);
	~DebugWindowClass();

	bool Initialize(int, int, int, int);
	void Shutdown();
	bool Render(int, int);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(int, int);
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;
	int vertex_count_, index_count_;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};

#endif