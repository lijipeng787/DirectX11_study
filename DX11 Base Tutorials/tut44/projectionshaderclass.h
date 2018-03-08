////////////////////////////////////////////////////////////////////////////////
// Filename: projectionshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _PROJECTIONSHADERCLASS_H_
#define _PROJECTIONSHADERCLASS_H_


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
// Class name: ProjectionShaderClass
////////////////////////////////////////////////////////////////////////////////
class ProjectionShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX view2;
		XMMATRIX projection2;
	};

	struct LightPositionBufferType
	{
		XMFLOAT3 lightPosition;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
	};

public:
	ProjectionShaderClass();
	ProjectionShaderClass(const ProjectionShaderClass&);
	~ProjectionShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*, const XMFLOAT4&, const XMFLOAT4&, const XMFLOAT3&, 
		const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView* );

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters( ID3D11DeviceContext*, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView*, const XMFLOAT4&, const XMFLOAT4&, const XMFLOAT3&,
		const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView* );
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* vertex_shader_;
	ID3D11PixelShader* pixel_shader_;
	ID3D11InputLayout* layout_;
	ID3D11SamplerState* sample_state_;
	ID3D11Buffer* matrix_buffer_;
	ID3D11Buffer* m_lightPositionBuffer;
	ID3D11Buffer* m_lightBuffer;
};

#endif