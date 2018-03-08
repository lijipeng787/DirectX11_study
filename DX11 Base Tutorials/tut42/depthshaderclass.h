
// Filename: depthshaderclass.h

#ifndef _DEPTHSHADERCLASS_H_
#define _DEPTHSHADERCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;



// Class name: DepthShaderClass

class DepthShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	DepthShaderClass();
	DepthShaderClass(const DepthShaderClass&);
	~DepthShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render( int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX& );

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters( const XMMATRIX&, const XMMATRIX&, const XMMATRIX& );
	void RenderShader(int);

private:
	ID3D11VertexShader* vertex_shader_;
	ID3D11PixelShader* pixel_shader_;
	ID3D11InputLayout* layout_;
	ID3D11Buffer* matrix_buffer_;
};

#endif