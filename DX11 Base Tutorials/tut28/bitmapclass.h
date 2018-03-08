
// Filename: bitmapclass.h

#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;



// Class name: BitmapClass

class SimpleMoveableSurface
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	SimpleMoveableSurface();
	SimpleMoveableSurface(const SimpleMoveableSurface&);
	~SimpleMoveableSurface();

	bool Initialize(ID3D11Device*, int, int, int, int);
	void Shutdown();
	bool Render(int, int);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(int, int);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;
	int vertex_count_, index_count_;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};

#endif