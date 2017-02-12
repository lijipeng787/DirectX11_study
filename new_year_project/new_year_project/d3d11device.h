#pragma once

#ifndef APPLICATION_HEADER_D3DCLASS_H
#define APPLICATION_HEADER_D3DCLASS_H

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include "status.h"
#include "shader.h"
#include "input_layout.h"
#include "buffer_config.h"
#include "sampler_state.h"

class VertexBufferConfig;

class IndexBufferConfig;

class ConstantBufferConfig;

typedef std::vector<VertexShader> VertexShaderVector;

typedef std::vector<PixelShader> PixelShaderVector;

typedef std::vector<ID3D11InputLayout*> InputLayoutVector;

typedef std::vector<ID3D11Buffer*> VertexBufferVector;

typedef std::vector<ID3D11Buffer*> IndexBufferVector;

typedef std::vector<ID3D11Buffer*> ConstantBufferVectors;

typedef std::vector<ID3D11SamplerState*> SamplerStateVector;

typedef std::vector<ID3D11Resource*> ShaderResourceVector;

typedef std::vector<ID3D11DepthStencilState*> DepthStencilVector;

typedef std::vector<ID3D11BlendState*> BlendStateVector;

typedef std::vector<ID3D11RasterizerState*> RasterizerState;

typedef std::vector<D3D11_VIEWPORT> ViewPortVector;

typedef std::vector<ID3D11RenderTargetView*> RenderTargetViewVector;

typedef std::vector<ID3D11DepthStencilView*> DepthStencilViewVector;

typedef std::unordered_map<LPCWSTR, int> VertexShaderNameTable;

typedef std::unordered_map<LPCWSTR, int> PixelShaderNameTable;

typedef std::unordered_map<LPCWSTR, int> InputLayoutNameTable;

typedef std::unordered_map <LPCWSTR, int> VertexBufferNameTable;

typedef std::unordered_map<LPCWSTR, int> IndexBufferNameTable;

typedef std::unordered_map<LPCWSTR, int> SamplerStateNameTable;

typedef std::unordered_map<LPCWSTR, int> ConstantBufferNameTable;

typedef std::unordered_map<LPCWSTR, int> ShaderResourceNameTable;

typedef std::pair<size_t, Status> Result;

class D3d11Device {
public:
	D3d11Device();

	D3d11Device(const D3d11Device& copy);

	~D3d11Device();

public:
	static D3d11Device* get_d3d_object();

	bool Initialize(
		int screen_width, 
		int screen_height,
		bool vsync, 
		HWND hwnd, 
		bool full_screen,
		float screen_depth, 
		float screen_near
		);

	void Shutdown();

	void BeginScene(
		float red,
		float green,
		float blue,
		float alpha
		);

	void EndScene();

	ID3D11Device* get_device();

	ID3D11DeviceContext* get_device_context();

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

	Result CreateVertexBuffer(
		const VertexBufferConfig vertex_buffer_config,
		const D3D11_SUBRESOURCE_DATA *vertex_init_data,
		const LPCWSTR vertex_buffer_name
		);

	Result CreateIndexBuffer(
		const IndexBufferConfig index_buffer_config,
		const D3D11_SUBRESOURCE_DATA *index_init_data,
		const LPCWSTR index_buffer_name
		);

	Result CreateConstantBuffer(
		const ConstantBufferConfig& constant_buffer_config,
		const D3D11_SUBRESOURCE_DATA *constant_init_data,
		const LPCWSTR constant_buffer_name
		);

	Result CreateSampler(
		const LPCWSTR sampler_state_name,
		SamplerState *sampler_state_desc,
		const UINT num_sampler_state_desc
		);

	Result CreateViewPort();

	Result CreateBlendState();

	Result CreateDepthStencilState();

	Result CreateRasterizerState();

	const VertexShader& GetVertexShaderByIndex(const int index)const;

	const VertexShader& GetVertexShaderByName(const LPCWSTR shader_name);

	const PixelShader& GetPixelShaderByIndex(const int index)const;

	const PixelShader& GetPixelShaderByName(const LPCWSTR shader_name);

	ID3D11InputLayout* GetInputLayoutByIndex(const int index)const;

	ID3D11InputLayout* GetInputLayoutByName(const LPCWSTR name);

	ID3D11Buffer* GetVertexBufferByIndex(const int index)const;

	ID3D11Buffer* GetVertexBufferByName(const LPCWSTR buffer_name);

	ID3D11Buffer* GetConstantBufferByIndex(const int index)const;
					 
	ID3D11Buffer* GetConstantBufferByName(const LPCWSTR buffer_name);

	ID3D11SamplerState* GetSamplerByIndex(const int index)const;

	ID3D11SamplerState* GetSamplerByName(const LPCWSTR name);

private:
	VertexShaderVector vertex_shader_container_;

	PixelShaderVector pixel_shader_container_;

	InputLayoutVector input_layout_container_;

	VertexBufferVector vertex_buffer_container_;

	IndexBufferVector index_buffer_container_;

	SamplerStateVector sampler_state_container_;

	ConstantBufferVectors constant_buffer_container_;

	VertexShaderNameTable vertex_shader_name_table_;

	PixelShaderNameTable pixel_shader_name_table_;

	InputLayoutNameTable input_layout_name_table_;

	VertexBufferNameTable vertex_buffer_name_table_;

	IndexBufferNameTable index_buffer_name_table_;

	SamplerStateNameTable sampler_state_name_table_;

	ConstantBufferNameTable constant_buffer_name_table_;

public:
	void get_projection_matrix(DirectX::XMMATRIX& projection_matrix);

	void get_world_matrix(DirectX::XMMATRIX& world_matrix);

	void get_orthogonality_matrix(DirectX::XMMATRIX& orthogonality_matrix);

	void get_videocard_info(char *videocard_name, int &videocard_memory_capacity);

	void TurnZBufferOn();

	void TurnZBufferOff();

	void TurnCullingOn();

	void TurnCullingOff();

	void TurnAlphaBlendingOn();

	void TurnAlphaBlendingOff();

	void TurnWireFrameOn();

	void TurnWireFrameOff();

	void TurnAlphaToCoverageBlendingOn();

	void TurnAlphaToCoverageBlendingOff();

private:
	static D3d11Device *d3d_object_;

	bool vsync_enable_ = false;

	int videocard_memory_capacity_ = 0;

	char videocard_description_[128] = { 0 };

	IDXGISwapChain *swap_chain = nullptr;

	ID3D11Device *device = nullptr;

	ID3D11DeviceContext *device_context = nullptr;

	ID3D11RenderTargetView *render_target_view = nullptr;

	ID3D11Texture2D *deptn_stencil_buffer = nullptr;

	ID3D11DepthStencilState *depth_stencil_state = nullptr;

	ID3D11DepthStencilState *stencil_no_depth_state = nullptr;

	ID3D11DepthStencilView *depth_stencil_view = nullptr;

	ID3D11RasterizerState *rasterizer_state = nullptr;

	ID3D11RasterizerState *rasterizer_state_no_culling = nullptr;

	ID3D11RasterizerState *rasterizer_state_wireframe = nullptr;

	DirectX::XMMATRIX projection_matrix = DirectX::XMMATRIX();

	DirectX::XMMATRIX orthoganlity_matrix = DirectX::XMMATRIX();

	DirectX::XMMATRIX world_matrix = DirectX::XMMATRIX();

	ID3D11BlendState *alpha_blending_state = nullptr;

	ID3D11BlendState *blending_no_alpha_state = nullptr;

	ID3D11BlendState *alpha_to_coverage_blending_state = nullptr;
};

#endif // !APPLICATION_HEADER_D3DCLASS_H