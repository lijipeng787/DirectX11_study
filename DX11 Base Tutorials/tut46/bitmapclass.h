
// Filename: bitmapclass.h

#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;





#include "textureclass.h"



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

	bool Initialize(ID3D11Device*, int, int, WCHAR*, WCHAR*, int, int);
	void Shutdown();
	bool Render(int, int);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
	ID3D11ShaderResourceView* GetGlowMap();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(int, int);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, WCHAR*, WCHAR*);
	void ReleaseTextures();

private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;
	int vertex_count_, index_count_;
	TextureClass* texture_;
	TextureClass* m_GlowMap;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};

#endif