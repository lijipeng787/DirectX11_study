#pragma once

#ifndef GRAPHICS_HEADER_RENDERABLE_H

#define GRAPHICS_HEADER_RENDERABLE_H

#include "common.h"

#define RESULTTEST(resource_handle,result)							\
		if (result.second.IsOk()) {									\
			resource_handle = result.first;							\
		}															\
		else {														\
			OutputDebugStringA(result.second.ToString().c_str());	\
			return false;											\
		}															\

class Renderable {
public:
	Renderable();

	Renderable(const Renderable& copy);

	virtual ~Renderable();

public:
	Result CreateVertexShader(
		LPCWSTR vertex_shader_filename,
		LPCSTR entry_point,
		LPCSTR target,
		LPCWSTR vertex_shader_name
		);

	Result CreatePixelShader(
		LPCWSTR pixel_shader_filename,
		LPCSTR entry_point,
		LPCSTR target,
		LPCWSTR pixel_shader_name
		);

	Result CreateInputLayout(
		LPCWSTR input_layout_name,
		InputLayoutDesc *input_layout_desces,
		UINT num_input_layout_desc,
		ID3D10Blob *associate_compiled_shader,
		LPCWSTR associate_shader_name
		);

	virtual Result CreateVertexBuffer(
		const VertexBufferConfig& buffer_config,
		const D3D11_SUBRESOURCE_DATA *vertex_init_daata,
		const LPCWSTR vertex_buffe_name
		);

	virtual Result CreateIndexBuffer(
		const IndexBufferConfig& buffer_config,
		const D3D11_SUBRESOURCE_DATA *index_init_daata,
		const LPCWSTR index_buffe_name
		);

	Result CreateConstantBuffer(
		const ConstantBufferConfig& buffer_config, 
		const D3D11_SUBRESOURCE_DATA *constant_init_daata, 
		const LPCWSTR constant_buffer_name
		);

	Result CreateSamplerState(
		const LPCWSTR sampler_state_name,
		SamplerState *sampler_state,
		const UINT num_sampler_state
		);

	ID3D11Buffer* GetVertexBufferByIndex(const int index)const;

	ID3D11Buffer* GetVertexBufferByName(const LPCWSTR buffer_name);

	ID3D11Buffer* GetConstantBuffersByIndex(const int index) const;

	ID3D11Buffer* GetConstantBuffersByName(const LPCWSTR buffer_name);

	const VertexShader& GetVertexShaderByIndex(const int index)const;

	const VertexShader& GetVertexShaderByName(const LPCWSTR shader_name);

	const PixelShader& GetPixelShaderByIndex(const int index)const;

	const PixelShader& GetPixelShaderByName(const LPCWSTR shader_name);

	ID3D11InputLayout* GetInputLayotByIndex(const int index)const;

	ID3D11InputLayout* GetInputLayotByName(const LPCWSTR name);

	ID3D11SamplerState* GetSamplerStateByIndex(const int index);

	ID3D11SamplerState* GetSamplerStateByName(const LPCWSTR name);

	void SetVertexShaderByIndex(const int index);

	void SetPixelShaderByIndex(const int index);

	void SetInputLayoutByIndex(const int index);

	void SetSamplerByIndex(
		const UINT slot,
		const UINT num_sampler,
		const int index
		);

	virtual void DrawIndexed(const int index_count);

	virtual void InitializeVertexShaderStage();

	virtual void InitializePixelShaderStage();

	virtual void InitializeInputAssemblerStage();

	virtual void InitializeOutputMergeStage();
};

#endif // !GRAPHICS_HEADER_RENDERABLE_H
