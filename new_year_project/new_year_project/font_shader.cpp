#include "font_shader.h"

FontShader::FontShader() {
}

FontShader::FontShader(const FontShader & copy){
}

FontShader::~FontShader(){
}

bool FontShader::Initialize(HWND hwnd){
	if (!(InitializeShadersAndInputLayout(hwnd, L"../Engine/font.vs", L"../Engine/font.ps"))) {
		return false;
	}
	if (!InitializeConstantBuffers()) {
		return false;
	}

	return true;
}

bool FontShader::InitializeGraphicsPipeline(
	DirectX::XMMATRIX world,
	DirectX::XMMATRIX view,
	DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture,
	DirectX::XMFLOAT4 pixel_color
	){

	if (!(SetGraphicsPipelineParameters(world, view, projection, srv_texture, pixel_color))) {
		return false;
	}
	SetShaderAndInputLayout();
	SetSamplerState();

	return true;
}

void FontShader::DrawIndexed(int index_count) {
	D3d11Device::get_d3d_object()->get_device_context()->DrawIndexed(index_count, 0, 0);
}

bool FontShader::InitializeShadersAndInputLayout(HWND hwnd, WCHAR * vs_filename, WCHAR * ps_filename){

	auto result = CreateVertexShader(vs_filename, "FontVertexShader", "vs_5_0", L"font_vertex_shader");
	if (result.second.IsOk()) {
		vertex_shader_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	result = CreatePixelShader(ps_filename, "FontPixelShader", "ps_5_0", L"font_pixel_shader");
	if (result.second.IsOk()) {
		pixel_shader_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	InputLayoutDesc input_lauout_desc;
	input_lauout_desc.InitializeInputElementDesc(2);
	D3D11_INPUT_ELEMENT_DESC input_desc[2]{ { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
											{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 } };
	input_lauout_desc.SetInputLayoutDesc(input_desc);
	
	auto associate_compiled_shader = GetVertexShaderByIndex(vertex_shader_).get_compiled_shader().get();
	
	result = CreateInputLayout(
		L"font_input_layout", 
		&input_lauout_desc, 
		input_lauout_desc.get_num_element(), 
		*associate_compiled_shader,
		nullptr
		);

	if (result.second.IsOk()) {
		input_layout_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	SamplerState sampler_state;

	result = CreateSamplerState(L"default_sampler_state", &sampler_state, 1);
	if (result.second.IsOk()) {
		sampler_state_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	return true;
}

bool FontShader::SetGraphicsPipelineParameters(DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture, DirectX::XMFLOAT4 pixel_color){
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

	auto pixel_color_buffer = GetConstantBuffersByIndex(pixel_color_buffer_);
	if (FAILED(device_context->Map(pixel_color_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_subresource_data))) {
		return false;
	}

	auto pixel_color_data = static_cast<PixelColorBufferType*>(map_subresource_data.pData);
	pixel_color_data->pixel_color = pixel_color;

	device_context->Unmap(pixel_color_buffer, 0);

	device_context->PSSetConstantBuffers(0, 1, &pixel_color_buffer);

	return true;
}

void FontShader::SetShaderAndInputLayout(){
	SetVertexShaderByIndex(vertex_shader_);
	SetPixelShaderByIndex(pixel_shader_);
	SetInputLayoutByIndex(input_layout_);
}

void FontShader::SetSamplerState(){
	SetSamplerByIndex(0, 1, sampler_state_);
}

bool FontShader::InitializeConstantBuffers(){
	ConstantBufferConfig buffer_config;
	buffer_config.SetBufferUsage(BufferUseage::kUsageDynamic);
	buffer_config.SetByteWidth(sizeof(MatrixBufferType));
	buffer_config.SetCpuAccessType(D3D11_CPU_ACCESS_WRITE);
	buffer_config.SetMiscFlags(0);
	buffer_config.SetStructreByteStride(0);
	auto result = CreateConstantBuffer(buffer_config, nullptr, L"font_matrix_buffer");
	if (result.second.IsOk()) {
		matrix_buffer_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	buffer_config.SetByteWidth(sizeof(PixelColorBufferType));
	result = CreateConstantBuffer(buffer_config, nullptr, L"pixel_color_buffer");
	if (result.second.IsOk()) {
		pixel_color_buffer_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}
	return true;
}
