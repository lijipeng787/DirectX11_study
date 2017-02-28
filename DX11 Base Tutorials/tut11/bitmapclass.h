#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>

class TextureClass;

class BitmapClass{
public:
	BitmapClass() {}
	
	BitmapClass(const BitmapClass& rhs) = delete;

	BitmapClass& operator=(const BitmapClass& rhs) = delete;
	
	~BitmapClass() {}
private:
	struct VertexType{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
	};
public:
	bool Initialize(ID3D11Device*, int, int, WCHAR*, int, int);

	void Shutdown();
	
	bool Render(ID3D11DeviceContext*, int, int);

	int GetIndexCount();
	
	ID3D11ShaderResourceView* GetTexture();
private:
	bool InitializeBuffers(ID3D11Device*);

	void ShutdownBuffers();
	
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	
	void ReleaseTexture();
private:
	ID3D11Buffer *m_vertexBuffer = nullptr, *m_indexBuffer = nullptr;

	int m_vertexCount = 0, m_indexCount = 0;
	
	TextureClass* m_Texture = nullptr;
	
	int m_screenWidth = 0, m_screenHeight = 0;
	
	int m_bitmapWidth = 0, m_bitmapHeight = 0;
	
	int m_previousPosX = 0, m_previousPosY = 0;
};

#endif