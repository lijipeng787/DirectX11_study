#ifndef _PROJECTIONSHADERCLASS_H_
#define _PROJECTIONSHADERCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class ProjectionShaderClass {
public:
	ProjectionShaderClass();

	ProjectionShaderClass(const ProjectionShaderClass&);
	
	~ProjectionShaderClass();
public:
	bool Initialize(HWND);

	void Shutdown();
	
	bool Render(
		int, const 
		XMMATRIX&, const XMMATRIX&, const XMMATRIX&, 
		ID3D11ShaderResourceView*, 
		const XMFLOAT4&, const XMFLOAT4&, const XMFLOAT3&,
		const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView*);
private:
	bool InitializeShader(HWND, WCHAR*, WCHAR*);

	void ShutdownShader();
	
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(
		const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, 
		ID3D11ShaderResourceView*, const XMFLOAT4&, 
		const XMFLOAT4&, const XMFLOAT3&,
		const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView*);
	
	void RenderShader(int);
private:
	ID3D11VertexShader* vertex_shader_;

	ID3D11PixelShader* pixel_shader_;
	
	ID3D11InputLayout* layout_;
	
	ID3D11SamplerState* sample_state_;
	
	ID3D11Buffer* matrix_buffer_;
	
	ID3D11Buffer* m_lightPositionBuffer;
	
	ID3D11Buffer* light_buffer_;
};

#endif