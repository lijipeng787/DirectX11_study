#include "modelclass.h"
#include "textureclass.h"
#include "../CommonFramework/DirectX11Device.h"

#include <DirectXMath.h>
#include <fstream>

using namespace std;
using namespace DirectX;

struct VertexType {
	XMFLOAT3 position;
	XMFLOAT2 texture;
};

struct ModelType {
	float x, y, z;
	float tu, tv;
	float nx, ny, nz;
};

bool ModelClass::Initialize(char* modelFilename, WCHAR* textureFilename1, WCHAR* textureFilename2, 
							WCHAR* textureFilename3)
{
	
	auto result = LoadModel(modelFilename);
	if(!result)
	{
		return false;
	}

	result = InitializeBuffers();
	if(!result)
	{
		return false;
	}

	result = LoadTextures(textureFilename1, textureFilename2, textureFilename3);
	if(!result)
	{
		return false;
	}

	return true;
}

void ModelClass::Shutdown()
{
	
	ReleaseTextures();

	ShutdownBuffers();
	
	ReleaseModel();	
}

void ModelClass::Render()
{
	RenderBuffers();	
}

int ModelClass::GetIndexCount()
{
	return index_count_;
}

bool ModelClass::InitializeBuffers()
{
	
	auto vertices = new VertexType[vertex_count_];
	if(!vertices)
	{
		return false;
	}

	auto indices = new unsigned long[index_count_];
	if(!indices)
	{
		return false;
	}

	for(int i=0; i<vertex_count_; i++)
	{
		vertices[i].position = XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
		vertices[i].texture = XMFLOAT2(model_[i].tu, model_[i].tv);
		indices[i] = i;
	}

	D3D11_BUFFER_DESC vertex_buffer_desc;

    vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = 0;
    vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertex_data;

    vertex_data.pSysMem = vertices;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

    auto result = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
	if(FAILED(result))
	{
		return false;
	}

	D3D11_BUFFER_DESC index_buffer_desc;

    index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_buffer_desc.CPUAccessFlags = 0;
    index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;

    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&index_buffer_desc, &indexData, &index_buffer_);
	if (FAILED(result)) {
		return false;
	}
	
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	return true;
}

void ModelClass::ShutdownBuffers()
{

	if(index_buffer_)
	{
		index_buffer_->Release();
		index_buffer_=nullptr;
	}
	
	if(vertex_buffer_)
	{
		vertex_buffer_->Release();
		vertex_buffer_=nullptr;
	}
}

void ModelClass::RenderBuffers()
{
	
	unsigned int stride = sizeof(VertexType); 
	unsigned int offset = 0;
    
	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool ModelClass::LoadTextures(WCHAR* textureFilename1, WCHAR* textureFilename2, WCHAR* textureFilename3)
{
	
	texture_1_ = new TextureClass();
	if(!texture_1_)
	{
		return false;
	}

	auto result = texture_1_->Initialize(textureFilename1);
	if(!result)
	{
		return false;
	}

	texture_2_ = new TextureClass();
	if(!texture_2_)
	{
		return false;
	}

	result = texture_2_->Initialize(textureFilename2);
	if(!result)
	{
		return false;
	}

	texture_3_ = new TextureClass();
	if(!texture_3_)
	{
		return false;
	}

	result = texture_3_->Initialize(textureFilename3);
	if(!result)
	{
		return false;
	}

	return true;
}

void ModelClass::ReleaseTextures() {

	if (texture_1_) {
		texture_1_->Shutdown();
		delete texture_1_;
		texture_1_ = 0;
	}

	if (texture_2_) {
		texture_2_->Shutdown();
		delete texture_2_;
		texture_2_ = 0;
	}

	if (texture_3_) {
		texture_3_->Shutdown();
		delete texture_3_;
		texture_3_ = 0;
	}
}

ID3D11ShaderResourceView* ModelClass::GetTexture1()
{
	return texture_1_->GetTexture();
}

ID3D11ShaderResourceView* ModelClass::GetTexture2()
{
	return texture_2_->GetTexture();
}

ID3D11ShaderResourceView* ModelClass::GetTexture3()
{
	return texture_3_->GetTexture();
}

bool ModelClass::LoadModel(char* filename)
{
	ifstream fin;
	fin.open(filename);
	if(fin.fail())
	{
		return false;
	}

	char input = 0;
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	fin >> vertex_count_;

	index_count_ = vertex_count_;

	model_ = new ModelType[vertex_count_];
	if(!model_)
	{
		return false;
	}
	
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	
	for(int i=0; i<vertex_count_; i++)
	{
		fin >> model_[i].x >> model_[i].y >> model_[i].z;
		fin >> model_[i].tu >> model_[i].tv;
		fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;
	}

	
	fin.close();

	return true;
}

void ModelClass::ReleaseModel() {

	if (model_) {
		delete[] model_;
		model_ = nullptr;
	}
}