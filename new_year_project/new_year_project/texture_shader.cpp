#include "texture_shader.h"

TextureShader::TextureShader() {
}

TextureShader::TextureShader(const TextureShader & copy){
}

TextureShader::~TextureShader(){
}

bool TextureShader::Initialize(HWND hwnd) {
	if (!(InitializeShadersAndInputLayout(hwnd, L"../Engine/texture.vs", L"../Engine/texture.ps"))) {
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

bool TextureShader::InitializeGraphicsPipeline(
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture
	){
	if (!(InitializeGraphicsPipelineParameters(world, view, projection, srv_texture))) {
		return false;
	}
	SetShadersAndInputLayout();

	return true;
}

void TextureShader::DrawIndexed(const int index){
	D3d11Device::get_d3d_object()->get_device_context()->DrawIndexed(index, 0, 0);
}

bool TextureShader::InitializeShadersAndInputLayout(HWND hwnd, WCHAR * vs_filename, WCHAR * ps_filename){

	RESULTTEST(vertex_shader_ , CreateVertexShader(vs_filename, "TextureVertexShader", "vs_5_0", L"texture_vertex_shader"));
	RESULTTEST(pixel_shader_ , CreatePixelShader(ps_filename, "TexturePixelShader", "ps_5_0", L"texture_pixel_shader"));

	D3D11_INPUT_ELEMENT_DESC element_desc[2] = { {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT ,0,0,D3D11_INPUT_PER_VERTEX_DATA ,0},
												{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT ,0,D3D11_APPEND_ALIGNED_ELEMENT ,D3D11_INPUT_PER_VERTEX_DATA ,0} };
	InputLayoutDesc input_layout_desc;
	input_layout_desc.InitializeInputElementDesc(2);
	input_layout_desc.SetInputLayoutDesc(element_desc);

	auto associate_compiled_shader = GetVertexShaderByIndex(vertex_shader_).get_compiled_shader().get();

	RESULTTEST(input_layout_ , CreateInputLayout(
		L"texture_input_layout",
		&input_layout_desc,
		input_layout_desc.get_num_element(),
		*associate_compiled_shader,
		nullptr
		));

	return true;
}

bool TextureShader::InitializeGraphicsPipelineParameters(
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection, 
	ID3D11ShaderResourceView * srv_texture
	){
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

	return true;
}

bool TextureShader::InitializeConstantBuffers(){
	ConstantBufferConfig buffer_desc;
	buffer_desc.SetBufferUsage(BufferUseage::kUsageDynamic);
	buffer_desc.SetByteWidth(sizeof(MatrixBufferType));
	buffer_desc.SetCpuAccessType(D3D11_CPU_ACCESS_WRITE);
	buffer_desc.SetMiscFlags(0);
	buffer_desc.SetStructreByteStride(0);

	RESULTTEST(matrix_buffer_ , CreateConstantBuffer(buffer_desc, nullptr, L"texture_matrix_buffer"));

	return true;
}

bool TextureShader::InitializeSampler(){
	SamplerState default_sampler;
	RESULTTEST(sampler_state_ , CreateSamplerState(L"default_sampler", &default_sampler, 1));

	return true;
}

void TextureShader::SetShadersAndInputLayout(){
	SetInputLayoutByIndex(input_layout_);
	SetVertexShaderByIndex(vertex_shader_);
	SetPixelShaderByIndex(pixel_shader_);
	SetSamplerByIndex(0, 1, sampler_state_);
}
