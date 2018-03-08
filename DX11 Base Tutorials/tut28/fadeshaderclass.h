////////////////////////////////////////////////////////////////////////////////
// Filename: fadeshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FADESHADERCLASS_H_
#define _FADESHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;


////////////////////////////////////////////////////////////////////////////////
// Class name: FadeShaderClass
////////////////////////////////////////////////////////////////////////////////
class FadeShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct FadeBufferType
	{
		float fadeAmount;
		XMFLOAT3 padding;
	};

public:
	FadeShaderClass();
	FadeShaderClass(const FadeShaderClass&);
	~FadeShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, float);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, float);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* vertex_shader_;
	ID3D11PixelShader* pixel_shader_;
	ID3D11InputLayout* layout_;
	ID3D11Buffer* matrix_buffer_;
	ID3D11SamplerState* sample_state_;
	ID3D11Buffer* m_fadeBuffer;
};

#endif