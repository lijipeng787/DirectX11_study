#include "terrain_shader.h"

TerrainShader::TerrainShader() {
}

TerrainShader::TerrainShader(const TerrainShader & copy){
}

TerrainShader::~TerrainShader(){
}

bool TerrainShader::Initialize(HWND hwnd){
	if (!(InitializeShadersAndInputLayout(hwnd, L"../Engine/terrain.vs", L"../Engine/terrain.ps"))) {
		return false;
	}
	if (!InitializeConstantBuffers()) {
		return false;
	}
	if (!InitializeSampler()) {
		return false;
	}

	return true;
}

bool TerrainShader::InitializeShadersAndInputLayout(HWND hwnd, WCHAR * vs_filename, WCHAR * ps_filename){

	RESULTTEST(vertex_shader_, CreateVertexShader(vs_filename, "TerrainVertexShader", "vs_5_0", L"terrain_vertex_shader"));
	RESULTTEST(pixel_shader_, CreatePixelShader(ps_filename, "TerrainPixelShader", "ps_5_0", L"terrain_pixel_shader"));

	D3D11_INPUT_ELEMENT_DESC element_desc[6]=
	{
		{ "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT ,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT ,D3D11_INPUT_PER_VERTEX_DATA ,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT ,D3D11_INPUT_PER_VERTEX_DATA ,0},
		{"BINORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT ,D3D11_INPUT_PER_VERTEX_DATA ,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT ,D3D11_INPUT_PER_VERTEX_DATA ,0}
	};
	InputLayoutDesc input_layout_desc;
	input_layout_desc.InitializeInputElementDesc(6);
	input_layout_desc.SetInputLayoutDesc(element_desc);

	auto associate_compiled_shader = GetVertexShaderByIndex(vertex_shader_).get_compiled_shader().get();

	RESULTTEST(input_layout_, CreateInputLayout(
		L"terrain_input_layout",
		&input_layout_desc,
		input_layout_desc.get_num_element(),
		*associate_compiled_shader,
		nullptr
		));

	return true;
}

bool TerrainShader::InitalizeGraphicsPipelineParameters(DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, ID3D11ShaderResourceView * srv_normalmap,
	DirectX::XMFLOAT3 light_direction, DirectX::XMFLOAT4 diffuse_color){
	
	using namespace DirectX;

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	D3D11_MAPPED_SUBRESOURCE map_subresource_data;
	ZeroMemory(&map_subresource_data, sizeof(map_subresource_data));
	auto matrix_buffer = GetConstantBuffersByIndex(matrix_buffer_);
	if (FAILED(device_context->Map(matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_subresource_data))) {
		return false;
	}

	auto data = static_cast<MatrixBufferType*>(map_subresource_data.pData);
	data->world = world;
	data->view = view;
	data->projection = projection;

	device_context->Unmap(matrix_buffer, 0);

	device_context->VSSetConstantBuffers(0, 1, &matrix_buffer);
	auto light_buffer = GetConstantBuffersByIndex(light_buffer_);
	if (FAILED(device_context->Map(light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_subresource_data))) {
		return false;
	}

	auto light_buffer_data = static_cast<LightBufferType*>(map_subresource_data.pData);
	light_buffer_data->light_direction = light_direction;
	light_buffer_data->diffuse_color = diffuse_color;
	light_buffer_data->padding = 0.0f;

	device_context->Unmap(light_buffer, 0);

	device_context->PSSetConstantBuffers(0, 1, &light_buffer);

	device_context->PSSetShaderResources(0, 1, &srv_texture);

	device_context->PSSetShaderResources(1, 1, &srv_normalmap);

	return true;
}

bool TerrainShader::InitializeConstantBuffers(){
	ConstantBufferConfig constant_buffer_desc;
	constant_buffer_desc.SetBufferUsage(BufferUseage::kUsageDynamic);
	constant_buffer_desc.SetByteWidth(sizeof(MatrixBufferType));
	constant_buffer_desc.SetCpuAccessType(D3D11_CPU_ACCESS_WRITE);
	constant_buffer_desc.SetMiscFlags(0);
	constant_buffer_desc.SetStructreByteStride(0);
	RESULTTEST(matrix_buffer_, CreateConstantBuffer(constant_buffer_desc, nullptr, L"terrain_matrix_buffer"));

	constant_buffer_desc.SetByteWidth(sizeof(LightBufferType));
	RESULTTEST(light_buffer_ , CreateConstantBuffer(constant_buffer_desc, nullptr, L"terrain_light_buffer"));

	return true;
}

bool TerrainShader::InitializeSampler(){
	SamplerState default_sampler;
	RESULTTEST(sampler_state_ , CreateSamplerState(L"default_sampler", &default_sampler, 1));

	return true;
}

void TerrainShader::SetShadersAndInputLayout(){
	SetInputLayoutByIndex(input_layout_);
	SetVertexShaderByIndex(vertex_shader_);
	SetPixelShaderByIndex(pixel_shader_);
	SetSamplerByIndex(0, 1, sampler_state_);
}

bool TerrainShader::InitializeGraphicsPipeline(
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection,
	ID3D11ShaderResourceView * srv_texture,
	ID3D11ShaderResourceView * srv_normalmap,
	DirectX::XMFLOAT3 light_direction, 
	DirectX::XMFLOAT4 diffuse_color
	) {
	
	if (!(InitalizeGraphicsPipelineParameters(world, view, projection,
		srv_texture, srv_normalmap, light_direction, diffuse_color))) {
		return false;
	}
	SetShadersAndInputLayout();

	return true;
}

void TerrainShader::DrawIndexed(const int index){
	D3d11Device::get_d3d_object()->get_device_context()->DrawIndexed(index, 0, 0);
}
