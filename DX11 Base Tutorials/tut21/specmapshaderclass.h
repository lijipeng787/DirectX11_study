
// Filename: specmapshaderclass.h

#ifndef _SPECMAPSHADERCLASS_H_
#define _SPECMAPSHADERCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <fstream>
using namespace std;
using namespace DirectX;



// Class name: SpecMapShaderClass

class SpecMapShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT4 specularColor;
		float specularPower;
		XMFLOAT3 lightDirection;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

public:
	SpecMapShaderClass();
	SpecMapShaderClass(const SpecMapShaderClass&);
	~SpecMapShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView**, const XMFLOAT3&, 
				const XMFLOAT4&, const XMFLOAT3&, const XMFLOAT4&, float );

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView**, const XMFLOAT3&, 
							 const XMFLOAT4&, const XMFLOAT3&, const XMFLOAT4&, float );
	void RenderShader(int);

private:
	ID3D11VertexShader* vertex_shader_;
	ID3D11PixelShader* pixel_shader_;
	ID3D11InputLayout* layout_;
	ID3D11Buffer* matrix_buffer_;
	ID3D11SamplerState* sample_state_;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_cameraBuffer;
};

#endif