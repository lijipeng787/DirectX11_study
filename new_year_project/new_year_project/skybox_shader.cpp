#include "skybox_shader.h"

SkyboxShader::SkyboxShader() {
}

SkyboxShader::SkyboxShader(const SkyboxShader & copy){
}

SkyboxShader::~SkyboxShader(){
}

bool SkyboxShader::Initialize(HWND hwnd){
	if (!(InitializeShadersAndInputLayout(hwnd, L"../Engine/skydome.vs", L"../Engine/skydome.ps"))) {
		return false;
	}
	if (!InitializeConstantBuffers()) {
		return false;
	}

	return true;
}

bool SkyboxShader::InitializeGraphicsPipeline(
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection,
	DirectX::XMFLOAT4 apex_color,
	DirectX::XMFLOAT4 center_color
	){

	if (!(InitializeGraphicsPipelineParameters(world, view, projection, apex_color, center_color))) {
		return false;
	}
	SetShadersAndInputLayout();

	return true;
}

void SkyboxShader::DrawIndexed(const int index){
	D3d11Device::get_d3d_object()->get_device_context()->DrawIndexed(index, 0, 0);
}

bool SkyboxShader::InitializeShadersAndInputLayout(HWND hwnd, WCHAR * vs_filename, WCHAR * ps_filename){
	
	RESULTTEST(vertex_shader_, CreateVertexShader(vs_filename, "SkyDomeVertexShader", "vs_5_0", L"skybox_vertex_shader"));
	RESULTTEST(pixel_shader_, CreatePixelShader(ps_filename, "SkyDomePixelShader", "ps_5_0", L"skybox_pixel_shader"));

	InputLayoutDesc input_layout_desc;
	input_layout_desc.InitializeInputElementDesc(1);
	D3D11_INPUT_ELEMENT_DESC element_desc[1] = { "POSITION" ,0,DXGI_FORMAT_R32G32B32_FLOAT ,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0 };
	input_layout_desc.SetInputLayoutDesc(element_desc);

	auto associate_compiled_shader = GetVertexShaderByIndex(vertex_shader_).get_compiled_shader().get();

	RESULTTEST(input_layout_, CreateInputLayout(
		L"skybox_inputlayout",
		&input_layout_desc,
		input_layout_desc.get_num_element(),
		*associate_compiled_shader,
		nullptr
		));

	return true;
}

bool SkyboxShader::InitializeGraphicsPipelineParameters(DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX projection, 
	DirectX::XMFLOAT4 apex_color, DirectX::XMFLOAT4 center_color){
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
	auto skybox_color_buffer = GetConstantBuffersByIndex(skybox_color_buffer_);
	if (FAILED(device_context->Map(skybox_color_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_subresource_data))) {
		return false;
	}

	auto skybox_data = static_cast<SkyboxColorBufferType*>(map_subresource_data.pData);
	skybox_data->apex_color = apex_color;
	skybox_data->center_color = center_color;

	device_context->Unmap(skybox_color_buffer, 0);

	device_context->PSSetConstantBuffers(0, 1, &skybox_color_buffer);

	return true;
}

bool SkyboxShader::InitializeConstantBuffers(){
	ConstantBufferConfig constant_buffer_desc;
	constant_buffer_desc.SetBufferUsage(BufferUseage::kUsageDynamic);
	constant_buffer_desc.SetByteWidth(sizeof(MatrixBufferType));
	constant_buffer_desc.SetCpuAccessType(D3D11_CPU_ACCESS_WRITE);
	constant_buffer_desc.SetMiscFlags(0);
	constant_buffer_desc.SetStructreByteStride(0);
	RESULTTEST(matrix_buffer_, CreateConstantBuffer(constant_buffer_desc, nullptr, L"skybox_matrix_buffer"));

	constant_buffer_desc.SetByteWidth(sizeof(SkyboxColorBufferType));
	RESULTTEST(skybox_color_buffer_, CreateConstantBuffer(constant_buffer_desc, nullptr, L"skyox_color_buffer"));

	return true;
}

void SkyboxShader::SetShadersAndInputLayout(){
	SetInputLayoutByIndex(input_layout_);
	SetVertexShaderByIndex(vertex_shader_);
	SetPixelShaderByIndex(pixel_shader_);
}
