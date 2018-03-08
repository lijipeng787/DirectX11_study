
// Filename: modelclass.cpp

#include "modelclass.h"


ModelClass::ModelClass()
{
	vertex_buffer_ = 0;
	index_buffer_ = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device)
{
	bool result;


	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	
	ShutdownBuffers();

	
}


void ModelClass::Render(ID3D11DeviceContext* device_context)
{
	
	RenderBuffers(device_context);

	
}


int ModelClass::GetIndexCount()
{
	return index_count_;
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
    D3D11_SUBRESOURCE_DATA vertex_date, indexData;
	HRESULT result;


	// Set the number of vertices in the vertex array.
	vertex_count_ = 3;

	// Set the number of indices in the index array.
	index_count_ = 3;

	
	vertices = new VertexType[vertex_count_];
	if(!vertices)
	{
		return false;
	}

	
	indices = new unsigned long[index_count_];
	if(!indices)
	{
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	vertices[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	vertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[2].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.

	
    vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = 0;
    vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	
    vertex_date.pSysMem = vertices;
	vertex_date.SysMemPitch = 0;
	vertex_date.SysMemSlicePitch = 0;

	
    result = device->CreateBuffer(&vertex_buffer_desc, &vertex_date, &vertex_buffer_);
	if(FAILED(result))
	{
		return false;
	}

	
    index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_buffer_desc.CPUAccessFlags = 0;
    index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	
	result = device->CreateBuffer(&index_buffer_desc, &indexData, &index_buffer_);
	if(FAILED(result))
	{
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
		index_buffer_ = 0;
	}

	
	if(vertex_buffer_)
	{
		vertex_buffer_->Release();
		vertex_buffer_ = 0;
	}

	
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* device_context)
{
	unsigned int stride;
	unsigned int offset;


	
	stride = sizeof(VertexType); 
	offset = 0;
    
	
	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

    
	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer.
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	
}