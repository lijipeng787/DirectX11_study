#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct MatrixBufferType;

class DepthShaderClass {
public:
	DepthShaderClass() {}

	DepthShaderClass(const DepthShaderClass& rhs) = delete;

	~DepthShaderClass() {}
public:
	bool Initialize(HWND);

	void Shutdown();

	bool Render(int, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);
private:
	bool InitializeShader(HWND, WCHAR*, WCHAR*);

	void ShutdownShader();

	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&);

	void RenderShader(int);
private:
	ID3D11VertexShader * vertex_shader_ = nullptr;

	ID3D11PixelShader* pixel_shader_ = nullptr;

	ID3D11InputLayout* layout_ = nullptr;

	ID3D11Buffer* matrix_buffer_ = nullptr;
};
