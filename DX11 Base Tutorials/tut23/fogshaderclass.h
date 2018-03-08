////////////////////////////////////////////////////////////////////////////////
// Filename: fogshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FOGSHADERCLASS_H_
#define _FOGSHADERCLASS_H_


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
// Class name: FogShaderClass
////////////////////////////////////////////////////////////////////////////////
class FogShaderClass
{
private:
	struct ConstantBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct FogBufferType
	{
		float fogStart;
		float fogEnd;
		float padding1, padding2;
	};

public:
	FogShaderClass();
	FogShaderClass(const FogShaderClass&);
	~FogShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, float, float);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, float, float);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* vertex_shader_;
	ID3D11PixelShader* pixel_shader_;
	ID3D11InputLayout* layout_;
	ID3D11Buffer* m_constantBuffer;
	ID3D11SamplerState* sample_state_;
	ID3D11Buffer* m_fogBuffer;
};

#endif