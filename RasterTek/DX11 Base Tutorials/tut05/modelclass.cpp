#include "modelclass.h"

Model::Model(){
}

Model::Model(const Model& copy){
}

Model::~Model(){
}

bool Model::Initialize(WCHAR* texture_filename){
	
	if(CHECK(InitializeBuffers())){
		return false;
	}

	if (CHECK(InitializeRootSignature())) {
		return false;
	}

	if (CHECK(InitializeGraphicsPipelineState())) {
		return false;
	}

	texture_ = std::make_shared<Texture>();

	if (CHECK(LoadTexture(texture_filename))) {
		return false;
	}

	return true;
}

void Model::SetVertexShader(const D3D12_SHADER_BYTECODE & verte_shader_bitecode){
	vertex_shader_bitecode_ = verte_shader_bitecode;
}

void Model::SetPixelShader(const D3D12_SHADER_BYTECODE & pixel_shader_bitcode){
	pixel_shader_bitcode_ = pixel_shader_bitcode;
}

const D3D12_VERTEX_BUFFER_VIEW & Model::GetVertexBufferView() const{
	return vertex_buffer_view_;
}

const D3D12_INDEX_BUFFER_VIEW & Model::GetIndexBufferView() const{
	return index_buffer_view_;
}

RootSignaturePtr Model::GetRootSignature() const{
	return root_signature_;
}

PipelineStateObjectPtr Model::GetPipelineStateObject() const{
	return pipeline_state_object_;
}

DescriptorHeapPtr Model::GetShaderRescourceView() const{
	return texture_->get_texture();
}

D3d12ResourcePtr Model::GetConstantBuffer() const{
	return constant_buffer_;
}

bool Model::UpdateConstantBuffer(
	DirectX::XMMATRIX& world, 
	DirectX::XMMATRIX& view , 
	DirectX::XMMATRIX& projection
	){
	
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = 0;
	UINT8 *data_begin = 0;
	if (FAILED(constant_buffer_->Map(0, &range, reinterpret_cast<void**>(&data_begin)))) {
		return false;
	}
	else {
		matrix_constant_data_.world_ = world;
		matrix_constant_data_.view_ = view;
		matrix_constant_data_.projection_ = projection;
		memcpy(data_begin, &matrix_constant_data_, sizeof(MatrixBufferType));
		constant_buffer_->Unmap(0, nullptr);
	}
	return true;
}

int Model::GetIndexCount(){
	return index_count_;
}

bool Model::InitializeBuffers()
{
	auto vertex_count = 3;
	index_count_ = 3;

	auto vertices = new VertexType[vertex_count];
	if (!vertices){
		return false;
	}

	auto indices = new uint16_t[index_count_];
	if (!indices){
		return false;
	}

	// Bottom left.
	vertices[0].position_ = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[0].texture_ = DirectX::XMFLOAT2(0.0f, 1.0f);
	// Top middle.
	vertices[1].position_ = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[1].texture_ = DirectX::XMFLOAT2(0.5f, 0.0f);
	// Bottom right.
	vertices[2].position_ = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertices[2].texture_ = DirectX::XMFLOAT2(1.0f, 1.0f);

	auto i = D3D12Device::GetD3d12DeviceInstance();
	if (FAILED(D3D12Device::GetD3d12DeviceInstance()->GetD3d12Device()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexType)*3),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertex_buffer_)
		))) {
		return false;
	}

	UINT8 *vertex_data_begin = nullptr;
	CD3DX12_RANGE read_range(0, 0);
	if (FAILED(vertex_buffer_->Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin)))) {
		return false;
	}

	memcpy(vertex_data_begin, vertices, sizeof(VertexType)*3);
	vertex_buffer_->Unmap(0, nullptr);

	vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
	vertex_buffer_view_.SizeInBytes = sizeof(VertexType)*3;
	vertex_buffer_view_.StrideInBytes = sizeof(VertexType);

	// Bottom left.
	indices[0] = 0;
	// Top middle.
	indices[1] = 1;
	// Bottom right.
	indices[2] = 2;
	
	if (FAILED(D3D12Device::GetD3d12DeviceInstance()->GetD3d12Device()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint16_t)*3),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&index_buffer_)
		))) {
		return false;
	}

	UINT8 *index_data_begin = nullptr;
	if (FAILED(index_buffer_->Map(0, &read_range, reinterpret_cast<void**>(&index_data_begin)))) {
		return false;
	}
	memcpy(index_data_begin, indices, sizeof(uint16_t)*3);
	index_buffer_->Unmap(0, nullptr);

	index_buffer_view_.BufferLocation = index_buffer_->GetGPUVirtualAddress();
	index_buffer_view_.SizeInBytes = sizeof(uint16_t)*3;
	index_buffer_view_.Format = DXGI_FORMAT_R16_UINT;

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC cbv_heap_desc = {};
	cbv_heap_desc.NumDescriptors = 1;
	cbv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (FAILED(D3D12Device::GetD3d12DeviceInstance()->GetD3d12Device()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(MatrixBufferType)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constant_buffer_)))) {
		return false;
	}

	ZeroMemory(&matrix_constant_data_, sizeof(MatrixBufferType));

	return true;
}

bool Model::InitializeGraphicsPipelineState() {

	D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
	pso_desc.pRootSignature = root_signature_.Get();
	pso_desc.VS = vertex_shader_bitecode_;
	pso_desc.PS = pixel_shader_bitcode_;
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pso_desc.SampleDesc.Count = 1;

	if (FAILED(D3D12Device::GetD3d12DeviceInstance()->GetD3d12Device()->CreateGraphicsPipelineState(
		&pso_desc,
		IID_PPV_ARGS(&pipeline_state_object_)
		))) {
		return false;
	}

	return true;
}

bool Model::InitializeRootSignature() {

	CD3DX12_DESCRIPTOR_RANGE descriptor_ranges_srv[1];
	descriptor_ranges_srv[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER root_parameters[2];
	root_parameters[0].InitAsDescriptorTable(1, &descriptor_ranges_srv[0], D3D12_SHADER_VISIBILITY_PIXEL);
	root_parameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	D3D12_STATIC_SAMPLER_DESC sampler_desc = {};
	sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 0;
	sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler_desc.MinLOD = 0.0f;
	sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
	sampler_desc.ShaderRegister = 0;
	sampler_desc.RegisterSpace = 0;
	sampler_desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC rootsignature_Layout(
		ARRAYSIZE(root_parameters), 
		root_parameters, 
		1, 
		&sampler_desc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		);

	ID3DBlob* signature_blob = nullptr;
	ID3DBlob *signature_error = nullptr;
	if (FAILED(D3D12SerializeRootSignature(
		&rootsignature_Layout,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signature_blob,
		&signature_error
		))) {
		return false;
	}

	if (FAILED(D3D12Device::GetD3d12DeviceInstance()->GetD3d12Device()->CreateRootSignature(
		0,
		signature_blob->GetBufferPointer(),
		signature_blob->GetBufferSize(),
		IID_PPV_ARGS(&root_signature_)
		))) {
		return false;
	}

	return true;
}

bool Model::LoadTexture(WCHAR *texture_filename){
	return texture_->set_texture(texture_filename);
}
