#include "skybox.h"

SkyBox::SkyBox() :skybox_model_(nullptr),
skybox_vertex_count_(0), skybox_index_count_(0),
vertex_buffer_(nullptr), index_buffer_(nullptr),
skybox_apex_color_(DirectX::XMFLOAT4(0.0f, 0.05f, 0.6f, 1.0f)),
skybox_center_color_(DirectX::XMFLOAT4(0.0f, 0.5f, 0.8f, 1.0f)) {
}

SkyBox::SkyBox(const SkyBox & copy){
}

SkyBox::~SkyBox(){
}

bool SkyBox::Initialize(){
	if (!(LoadSkyboxModel("../Engine/data/skydome/skydome.txt"))) {
		return false;
	}

	if (!(InitializeBuffers())) {
		return false;
	}

	return true;
}

void SkyBox::Shutdown(){
	ReleaseBuffers();
	ReleaseSkyboxModel();
}

void SkyBox::Render(){
	RenderBuffers();
}

int SkyBox::GetIndexCount(){
	return skybox_index_count_;
}

DirectX::XMFLOAT4 SkyBox::get_skybox_apex_color(){
	return skybox_apex_color_;
}

DirectX::XMFLOAT4 SkyBox::get_skybox_center_color(){
	return skybox_center_color_;
}

bool SkyBox::LoadSkyboxModel(char * model_filename){

	std::ifstream fin;
	fin.open(model_filename);

	if (fin.fail())
		return false;

	auto read_bit = ' ';
	fin.get(read_bit);
	while (':' != read_bit) {
		fin.get(read_bit);
	}
	fin >> skybox_vertex_count_;
	skybox_index_count_ = skybox_vertex_count_;

	skybox_model_ = new SkyBoxModelType[skybox_vertex_count_];
	if (!skybox_model_)
		return false;

	fin.get(read_bit);
	while (':' != read_bit) {
		fin.get(read_bit);
	}
	fin.get(read_bit);
	fin.get(read_bit);

	for (int i = 0; i < skybox_vertex_count_; ++i) {
		fin >> skybox_model_[i].x >> skybox_model_[i].y >> skybox_model_[i].z;
		fin >> skybox_model_[i].tu >> skybox_model_[i].tv;
		fin >> skybox_model_[i].nx >> skybox_model_[i].ny >> skybox_model_[i].nz;
	}
	
	fin.close();

	return true;
}

void SkyBox::ReleaseSkyboxModel(){
	if (skybox_model_) {
		delete[] skybox_model_;
		skybox_model_ = nullptr;
	}
}

bool SkyBox::InitializeBuffers(){
	using namespace DirectX;

	auto device = D3d11Device::get_d3d_object()->get_device();

	auto vertices = new VertexType[skybox_vertex_count_];
	if (!vertices)
		return false;

	auto indices = new unsigned long[skybox_index_count_];
	if (!indices)
		return false;

	for (int i = 0; i < skybox_vertex_count_; ++i) {
		vertices[i].position = XMFLOAT3(skybox_model_[i].x, skybox_model_[i].y, skybox_model_[i].z);
		indices[i] = i;
	}

	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(buffer_desc));

	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(VertexType) * skybox_vertex_count_;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA buffer_data;
	ZeroMemory(&buffer_data, sizeof(buffer_data));

	buffer_data.pSysMem = vertices;
	buffer_data.SysMemPitch = 0;
	buffer_data.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &vertex_buffer_))) {
		return false;
	}

	buffer_desc.ByteWidth = sizeof(unsigned long)*skybox_index_count_;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	buffer_data.pSysMem = indices;
	buffer_data.SysMemPitch = 0;
	buffer_data.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &index_buffer_))) {
		return false;
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void SkyBox::ReleaseBuffers(){
	if (vertex_buffer_) {
		vertex_buffer_->Release();
		vertex_buffer_ = nullptr;
	}

	if (index_buffer_) {
		index_buffer_->Release();
		index_buffer_ = nullptr;
	}
}

void SkyBox::RenderBuffers(){
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
