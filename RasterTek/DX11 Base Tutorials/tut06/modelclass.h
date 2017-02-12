#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <memory>
#include <DirectXMath.h>
#include "textureclass.h"

class Model{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT2 texture_;
		DirectX::XMFLOAT3 normal_;
	};

	struct MatrixBufferType {
		DirectX::XMMATRIX world_;
		DirectX::XMMATRIX view_;
		DirectX::XMMATRIX projection_;
		DirectX::XMMATRIX padding_;
	};

	struct LightType {
		DirectX::XMFLOAT4 color_;
		DirectX::XMFLOAT3 direction_;
		float padding_;
	};

public:
	Model();

	explicit Model(const Model& copy);
	
	~Model();

public:
	bool Initialize(WCHAR *texture_filename);

	void SetVertexShader(const D3D12_SHADER_BYTECODE& verte_shader_bitecode);

	void SetPixelShader(const D3D12_SHADER_BYTECODE& pixel_shader_bitcode);

	UINT GetIndexCount()const;

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView()const;

	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView()const;

	RootSignaturePtr GetRootSignature()const;

	PipelineStateObjectPtr GetPipelineStateObject()const;

	DescriptorHeapPtr GetShaderRescourceView()const;

	D3d12ResourcePtr GetMatrixConstantBuffer()const;

	D3d12ResourcePtr GetLightConstantBuffer()const;

	bool UpdateMatrixConstant(
		DirectX::XMMATRIX& world,
		DirectX::XMMATRIX& view,
		DirectX::XMMATRIX& projection
		);

	bool UpdateLightConstant(
		DirectX::XMFLOAT3& direction,
		DirectX::XMFLOAT4& color
		);

private:
	bool InitializeBuffers();

	bool InitializeRootSignature();

	bool InitializeGraphicsPipelineState();

	bool LoadTexture(WCHAR *texture_filename);
	
private:
	D3d12ResourcePtr vertex_buffer_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_ = {};

	D3d12ResourcePtr index_buffer_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW index_buffer_view_ = {};

	D3d12ResourcePtr matrix_constant_ = nullptr;

	MatrixBufferType matrix_constant_data_ = {};

	D3d12ResourcePtr light_constant_ = nullptr;

	LightType light_constant_data_ = {};

	RootSignaturePtr root_signature_ = nullptr;

	D3D12_SHADER_BYTECODE vertex_shader_bitecode_ = {};

	D3D12_SHADER_BYTECODE pixel_shader_bitecode_ = {};

	PipelineStateObjectPtr pipeline_state_object_ = nullptr;

private:
	UINT index_count_ = 0;

	std::shared_ptr<Texture> texture_ = nullptr;
};

#endif