#ifndef _GLOWMAPSHADERCLASS_H_
#define _GLOWMAPSHADERCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

class GlowMapShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

public:
	GlowMapShaderClass();
	GlowMapShaderClass(const GlowMapShaderClass&);
	~GlowMapShaderClass();

	bool Initialize(HWND);
	void Shutdown();
	bool Render(int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, ID3D11ShaderResourceView*);

private:
	bool InitializeShader(HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, ID3D11ShaderResourceView*);
	void RenderShader(int);

private:
	ID3D11VertexShader* vertex_shader_;
	ID3D11PixelShader* pixel_shader_;
	ID3D11InputLayout* layout_;
	ID3D11Buffer* matrix_buffer_;
	ID3D11SamplerState* sample_state_;
};

#endif