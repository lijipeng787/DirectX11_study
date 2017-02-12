#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <DirectXMath.h>
#include <fstream>
#include <memory>
#include "d3dclass.h"
#include "textureclass.h"

class Model {
private:
	struct VertexType {
		DirectX::XMFLOAT3 position_;
		DirectX::XMFLOAT2 texture_;
		DirectX::XMFLOAT3 normal_;
	};

	struct ModelType {
		float x_, y_, z_;
		float tu_, tv_;
		float nx_, ny_, nz_;
	};

	struct MatrixBufferType {
		DirectX::XMMATRIX world_;
		DirectX::XMMATRIX view_;
		DirectX::XMMATRIX projection_;
		DirectX::XMMATRIX padding_;
	};

	struct LightType {
		DirectX::XMFLOAT4 ambient_color_;
		DirectX::XMFLOAT4 diffuse_color_;
		DirectX::XMFLOAT3 direction_;
		float padding_;
	};

public:
	Model();

	explicit Model(const Model& copy);

	~Model();

public:
	bool Initialize(WCHAR *model_filename, WCHAR *texture_filename);

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
		DirectX::XMFLOAT4& ambient_color,
		DirectX::XMFLOAT4& diffuse_color,
		DirectX::XMFLOAT3& direction
		);

private:
	bool LoadModel(WCHAR *filename);

	bool InitializeBuffers();

	bool InitializeRootSignature();

	bool InitializeGraphicsPipelineState();

	bool LoadTexture(WCHAR *texture_filename);

private:
	D3d12ResourcePtr vertex_buffer_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_ = {};

	D3d12ResourcePtr index_buffer_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW index_buffer_view_ = {};

	D3d12ResourcePtr matrix_constant_buffer_ = nullptr;

	MatrixBufferType matrix_constant_data_ = {};

	D3d12ResourcePtr light_constant_buffer_ = nullptr;

	LightType light_constant_data_ = {};

	RootSignaturePtr root_signature_ = nullptr;

	PipelineStateObjectPtr pipeline_state_object_ = nullptr;

	D3D12_SHADER_BYTECODE vertex_shader_bitecode_ = {};

	D3D12_SHADER_BYTECODE pixel_shader_bitecode_ = {};

private:
	UINT vertex_count_ = 0;

	UINT index_count_ = 0;

	ModelType *tem_model_ = nullptr;

	std::shared_ptr<Texture> texture_ = nullptr;
};

#endif