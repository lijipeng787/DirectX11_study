////////////////////////////////////////////////////////////////////////////////
// Filename: horizontalblurshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _HORIZONTALBLURSHADERCLASS_H_
#define _HORIZONTALBLURSHADERCLASS_H_


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
// Class name: HorizontalBlurShaderClass
////////////////////////////////////////////////////////////////////////////////
class HorizontalBlurShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct ScreenSizeBufferType
	{
		float screenWidth;
		XMFLOAT3 padding;
	};

public:
	HorizontalBlurShaderClass();
	HorizontalBlurShaderClass(const HorizontalBlurShaderClass&);
	~HorizontalBlurShaderClass();

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
	ID3D11SamplerState* sample_state_;
	ID3D11Buffer* matrix_buffer_;
	ID3D11Buffer* m_screenSizeBuffer;
};

#endif