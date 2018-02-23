#ifndef _TEXTURESHADERCLASS_H_
#define _TEXTURESHADERCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

using namespace std;
using namespace DirectX;

class TextureShader{
public:
	TextureShader() {}
	
	TextureShader(const TextureShader& rhs) = delete;
	
	~TextureShader() {}
public:
	bool Initialize(HWND hWnd);
	
	void Shutdown();

	bool Render(int, const XMMATRIX&, const XMMATRIX&, const XMMATRIX&, ID3D11ShaderResourceView*);
private:
	bool InitializeShader(HWND, WCHAR*, WCHAR*);

	void ShutdownShader();
	
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(const XMMATRIX&, const XMMATRIX&, const XMMATRIX&,  ID3D11ShaderResourceView*);
	
	void RenderShader(int);
private:
	ID3D11VertexShader * vertex_shader_ = nullptr;
	
	ID3D11PixelShader* pixel_shader_ = nullptr;
	
	ID3D11InputLayout* input_layout_ = nullptr;
	
	ID3D11Buffer* matrix_buffer_ = nullptr;
	
	ID3D11SamplerState* sample_state_ = nullptr;
};

#endif