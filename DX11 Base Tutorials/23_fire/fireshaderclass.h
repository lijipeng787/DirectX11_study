#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct MatrixBufferType;
struct NoiseBufferType;
struct DistortionBufferType;

class FireShaderClass {
public:
	FireShaderClass() {}

	FireShaderClass(const FireShaderClass& rhs) = delete;

	~FireShaderClass() {}
public:
	bool Initialize(HWND);

	void Shutdown();

	bool Render(int,
				const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&,
				ID3D11ShaderResourceView*, ID3D11ShaderResourceView*,
				ID3D11ShaderResourceView*, float,
				const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&,
				const DirectX::XMFLOAT2&, const DirectX::XMFLOAT2&,
				const DirectX::XMFLOAT2&, float, float);
private:
	bool InitializeShader(HWND, WCHAR*, WCHAR*);

	void ShutdownShader();

	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(
		const DirectX::XMMATRIX&, const DirectX::XMMATRIX&, const DirectX::XMMATRIX&,
		ID3D11ShaderResourceView*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*,
		float, const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&,
		const DirectX::XMFLOAT2&, const DirectX::XMFLOAT2&, const DirectX::XMFLOAT2&,
		float, float);

	void RenderShader(int);
private:
	ID3D11VertexShader * vertex_shader_ = nullptr;

	ID3D11PixelShader* pixel_shader_ = nullptr;

	ID3D11InputLayout* layout_ = nullptr;

	ID3D11Buffer* matrix_buffer_ = nullptr, *noise_buffer_ = nullptr, *distortion_buffer_ = nullptr;

	ID3D11SamplerState* sample_state_ = nullptr, *sample_state_2_ = nullptr;
};