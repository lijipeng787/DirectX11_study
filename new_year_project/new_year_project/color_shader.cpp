#include "color_shader.h"

ColorShader::ColorShader() {
}

ColorShader::ColorShader(const ColorShader & copy){
}

ColorShader::~ColorShader(){
}

bool ColorShader::Initialize(HWND hwnd) {
	if (!(InitializeShadersAndInputLayout(hwnd, L"../Engine/color.vs", L"../Engine/color.ps"))) {
		return false;
	}
	if (!InitializeConstantBuffers()) {
		return false;
	}

	return true;
}

bool ColorShader::InitializeGraphicsPipeline(DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection) {
	if (!(SetGraphicsPipelineParameters(world, view, projection))) {
		return false;
	}
	SetShadersAndInputLayout();

	return true;
}

bool ColorShader::InitializeShadersAndInputLayout(HWND hwnd, WCHAR * vs_filename, WCHAR * ps_filename) {

	auto result = CreateVertexShader(vs_filename, "ColorVertexShader", "vs_5_0", L"color_vertex_shader");
	if (result.second.IsOk()) {
		vertex_shader_ = result.first;
	}
	else{
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	result = CreatePixelShader(ps_filename, "ColorPixelShader", "ps_5_0", L"color_pixel_shader");
	if (result.second.IsOk()) {
		pixel_shader_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	InputLayoutDesc input_layout_desc_object;
	input_layout_desc_object.InitializeInputElementDesc(2);
	D3D11_INPUT_ELEMENT_DESC input_desc[2]{ {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
											{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0} };
	input_layout_desc_object.SetInputLayoutDesc(input_desc);

	auto associate_vertex_shader = GetVertexShaderByIndex(vertex_shader_).get_compiled_shader().get();
	result = 
		D3d11Device::get_d3d_object()->CreateInputLayout(
		L"color_shader",
		&input_layout_desc_object,
		input_layout_desc_object.get_num_element(),
		*associate_vertex_shader,
		nullptr
		);
	if (result.second.IsOk()) {
		input_layout_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	return true;
}

bool ColorShader::InitializeConstantBuffers(){
	
	ConstantBufferConfig buffer_config;
	buffer_config.SetBufferUsage(BufferUseage::kUsageDynamic);
	buffer_config.SetByteWidth(sizeof(MatrixBufferType));
	buffer_config.SetCpuAccessType(D3D11_CPU_ACCESS_WRITE);
	buffer_config.SetMiscFlags(0);
	buffer_config.SetStructreByteStride(0);

	auto result = CreateConstantBuffer(buffer_config, nullptr, L"color_matrix_buffer");
	if (result.second.IsOk()) {
		matrix_buffer_ = result.first;
	}
	else {
		OutputDebugStringA(result.second.ToString().c_str());
		return false;
	}

	return true;
}

bool ColorShader::SetGraphicsPipelineParameters(DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection) {
	using namespace DirectX;

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	auto matrix_buffer = GetConstantBuffersByIndex(matrix_buffer_);

	D3D11_MAPPED_SUBRESOURCE map_subresource_data;
	ZeroMemory(&map_subresource_data, sizeof(map_subresource_data));
	if (FAILED(device_context->Map(matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_subresource_data))) {
		return false;
	}

	auto data = static_cast<MatrixBufferType*>(map_subresource_data.pData);
	data->world = world;
	data->view = view;
	data->projection = projection;

	device_context->Unmap(matrix_buffer, 0);

	device_context->VSSetConstantBuffers(0, 1, &matrix_buffer);

	return true;
}

void ColorShader::SetShadersAndInputLayout() {
	SetInputLayoutByIndex(this->input_layout_);
	SetVertexShaderByIndex(this->vertex_shader_);
	SetPixelShaderByIndex(this->pixel_shader_);
}

void ColorShader::DrawIndexed(int index_count){
	D3d11Device::get_d3d_object()->get_device_context()->DrawIndexed(index_count, 0, 0);
}
