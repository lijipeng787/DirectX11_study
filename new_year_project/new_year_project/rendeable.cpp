#include "rendeable.h"

Renderable::Renderable(){
}

Renderable::Renderable(const Renderable & copy){
}

Renderable::~Renderable(){
}

Result Renderable::CreateVertexShader(
	LPCWSTR vertex_shader_filename, 
	LPCSTR entry_point, 
	LPCSTR target, 
	LPCWSTR vertex_shader_name
	){
	return D3d11Device::get_d3d_object()->CreateVertexShader(vertex_shader_filename, entry_point, target, vertex_shader_name);
}

Result Renderable::CreatePixelShader(
	LPCWSTR pixel_shader_filename, 
	LPCSTR entry_point, 
	LPCSTR target, 
	LPCWSTR pixel_shader_name
	){
	return D3d11Device::get_d3d_object()->CreatePixelShader(pixel_shader_filename, entry_point, target, pixel_shader_name);
}

Result Renderable::CreateInputLayout(
	LPCWSTR input_layout_name, 
	InputLayoutDesc * input_layout_desces, 
	UINT num_input_layout_desc, 
	ID3D10Blob * associate_compiled_shader, 
	LPCWSTR associate_shader_name
	){

	return D3d11Device::get_d3d_object()->CreateInputLayout(
		input_layout_name, 
		input_layout_desces, 
		num_input_layout_desc, 
		associate_compiled_shader, 
		associate_shader_name
		);
}

Result Renderable::CreateVertexBuffer(
	const VertexBufferConfig & buffer_config,
	const D3D11_SUBRESOURCE_DATA *vertex_init_daata,
	const LPCWSTR vertex_buffe_name
	) {

	return D3d11Device::get_d3d_object()->CreateVertexBuffer(buffer_config, vertex_init_daata, vertex_buffe_name);
}

Result Renderable::CreateIndexBuffer(
	const IndexBufferConfig & buffer_config,
	const D3D11_SUBRESOURCE_DATA *index_init_daata,
	const LPCWSTR index_buffe_name
	) {

	return D3d11Device::get_d3d_object()->CreateIndexBuffer(buffer_config, index_init_daata, index_buffe_name);
}

Result Renderable::CreateConstantBuffer(
	const ConstantBufferConfig & buffer_config,
	const D3D11_SUBRESOURCE_DATA * constant_init_daata,
	const LPCWSTR constant_buffer_name
	) {
	return D3d11Device::get_d3d_object()->CreateConstantBuffer(buffer_config, constant_init_daata, constant_buffer_name);
}

Result Renderable::CreateSamplerState(
	const LPCWSTR sampler_state_name,
	SamplerState * sampler_state,
	const UINT num_sampler_state
	) {

	return D3d11Device::get_d3d_object()->CreateSampler(sampler_state_name, sampler_state, num_sampler_state);
}

ID3D11Buffer * Renderable::GetVertexBufferByIndex(const int index) const{
	return D3d11Device::get_d3d_object()->GetVertexBufferByIndex(index);
}

ID3D11Buffer * Renderable::GetVertexBufferByName(const LPCWSTR buffer_name){
	return D3d11Device::get_d3d_object()->GetVertexBufferByName(buffer_name);
}

ID3D11Buffer * Renderable::GetConstantBuffersByIndex(const int index) const{
	return D3d11Device::get_d3d_object()->GetConstantBufferByIndex(index);
}

ID3D11Buffer * Renderable::GetConstantBuffersByName(const LPCWSTR buffer_name){
	return D3d11Device::get_d3d_object()->GetConstantBufferByName(buffer_name);
}

const VertexShader& Renderable::GetVertexShaderByIndex(const int index) const{
	return D3d11Device::get_d3d_object()->GetVertexShaderByIndex(index);
}

const VertexShader& Renderable::GetVertexShaderByName(const LPCWSTR shader_name){
	return D3d11Device::get_d3d_object()->GetVertexShaderByName(shader_name);
}

const PixelShader& Renderable::GetPixelShaderByIndex(const int index) const{
	return D3d11Device::get_d3d_object()->GetPixelShaderByIndex(index);
}

const PixelShader& Renderable::GetPixelShaderByName(const LPCWSTR shader_name){
	return D3d11Device::get_d3d_object()->GetPixelShaderByName(shader_name);
}

ID3D11InputLayout * Renderable::GetInputLayotByIndex(const int index) const{
	return D3d11Device::get_d3d_object()->GetInputLayoutByIndex(index);
}

ID3D11InputLayout * Renderable::GetInputLayotByName(const LPCWSTR name){
	return D3d11Device::get_d3d_object()->GetInputLayoutByName(name);
}

ID3D11SamplerState * Renderable::GetSamplerStateByIndex(const int index){
	return D3d11Device::get_d3d_object()->GetSamplerByIndex(index);
}

ID3D11SamplerState * Renderable::GetSamplerStateByName(const LPCWSTR name){
	return D3d11Device::get_d3d_object()->GetSamplerByName(name);
}

void Renderable::SetVertexShaderByIndex(const int index){
	auto vertex_shader(GetVertexShaderByIndex(index));
	auto device_context = D3d11Device::get_d3d_object()->get_device_context();
	auto state = vertex_shader.get_vertex_shader().get();
	device_context->VSSetShader(*state, nullptr, 0);
}

void Renderable::SetPixelShaderByIndex(const int index){
	auto pixel_shader(GetPixelShaderByIndex(index));
	auto device_context = D3d11Device::get_d3d_object()->get_device_context();
	auto state = pixel_shader.get_pixel_shader().get();
	device_context->PSSetShader(*state, nullptr, 0);
}

void Renderable::SetInputLayoutByIndex(const int index){
	auto input_layout = GetInputLayotByIndex(index);
	auto device_context = D3d11Device::get_d3d_object()->get_device_context();
	device_context->IASetInputLayout(input_layout);
}

void Renderable::SetSamplerByIndex(
	const UINT slot, 
	const UINT num_sampler, 
	const int index
	){
	auto sampler_state = GetSamplerStateByIndex(index);
	auto device_context = D3d11Device::get_d3d_object()->get_device_context();
	device_context->PSSetSamplers(slot, num_sampler, &sampler_state);
}

void Renderable::DrawIndexed(int index_count){
}

void Renderable::InitializeVertexShaderStage(){
}

void Renderable::InitializePixelShaderStage(){
}

void Renderable::InitializeInputAssemblerStage(){
}

void Renderable::InitializeOutputMergeStage(){
}
