#include "light_shader.h"

LightShader::LightShader() {
}

LightShader::LightShader(const LightShader & copy){
}

LightShader::~LightShader(){
}

bool LightShader::Initialize(HWND hwnd){
	if (!(SetShadersAndInputLayout(hwnd, L"../Engine/light.vs", L"../Engine/light.ps"))) {
		return false;
	}
	if (!InitializeConstantBuffers()) {
		return false;
	}

	return true;
}

bool LightShader::InitializeGraphicsPipeline(
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, 
	DirectX::XMFLOAT3 light_direction, 
	DirectX::XMFLOAT4 light_diffuse_color
	){
	if (!(SetGraphicsPipelineParameters(world, view, projection, srv_texture, light_direction, light_diffuse_color))) {
		return false;
	}
	SetShaderAndInputLayout();
	SetSampler();

	return true;
}

void LightShader::DrawIndexed(const int index){
	D3d11Device::get_d3d_object()->get_device_context()->DrawIndexed(index,0,0);
}

bool LightShader::SetShadersAndInputLayout(HWND hwnd, WCHAR * vs_filename, WCHAR * ps_filename){

	SamplerState default_sampler_state;
	RESULTTEST(sampler_state_, CreateSamplerState(L"default_sampler_state", &default_sampler_state, 1));

	RESULTTEST(vertex_shader_, CreateVertexShader(vs_filename, "LightVertexShader", "vs_5_0", L"light_vertex_shader"));
	RESULTTEST(pixel_shader_, CreatePixelShader(ps_filename, "LightPixelShader", "ps_5_0", L"light_pixel_shader"));

	InputLayoutDesc input_lauout_desc;
	input_lauout_desc.InitializeInputElementDesc(3);
	D3D11_INPUT_ELEMENT_DESC input_desc[3]{ { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
										{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }, 
										{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 } };
	input_lauout_desc.SetInputLayoutDesc(input_desc);

	auto associate_compiled_shader = GetVertexShaderByIndex(vertex_shader_).get_compiled_shader().get();

	RESULTTEST(input_layout_, CreateInputLayout(
		L"light_input_layout",
		&input_lauout_desc,
		input_lauout_desc.get_num_element(),
		*associate_compiled_shader,
		nullptr
		));

	return true;
}

bool LightShader::SetGraphicsPipelineParameters(DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, DirectX::XMFLOAT3 light_direction, DirectX::XMFLOAT4 light_diffuse_color){
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

	device_context->PSSetShaderResources(0, 1, &srv_texture);

	auto light_buffer = GetConstantBuffersByIndex(light_buffer_);
	if (FAILED(device_context->Map(light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_subresource_data))) {
		return false;
	}

	auto light_data = static_cast<LightBufferType*>(map_subresource_data.pData);
	light_data->diffuser_color = light_diffuse_color;
	light_data->light_direction = light_direction;
	light_data->padding = 0.0f;

	device_context->Unmap(light_buffer, 0);

	device_context->PSSetConstantBuffers(0, 1, &light_buffer);

	return true;
}

bool LightShader::InitializeConstantBuffers() {
	ConstantBufferConfig constant_buffer_desc;

	constant_buffer_desc.SetBufferUsage(BufferUseage::kUsageDynamic);
	constant_buffer_desc.SetByteWidth(sizeof(MatrixBufferType));
	constant_buffer_desc.SetCpuAccessType(D3D11_CPU_ACCESS_WRITE);
	constant_buffer_desc.SetMiscFlags(0);
	constant_buffer_desc.SetStructreByteStride(0);

	RESULTTEST(matrix_buffer_, CreateConstantBuffer(constant_buffer_desc, nullptr, L"light_matrix_buffer"));

	constant_buffer_desc.SetByteWidth(sizeof(LightBufferType));

	RESULTTEST(light_buffer_, CreateConstantBuffer(constant_buffer_desc, nullptr, L"light_buffer"));

	return true;
}

void LightShader::SetShaderAndInputLayout(){
	SetInputLayoutByIndex(input_layout_);
	SetVertexShaderByIndex(vertex_shader_);
	SetPixelShaderByIndex(pixel_shader_);
}

void LightShader::SetSampler(){
	SetSamplerByIndex(0, 1, sampler_state_);
}

