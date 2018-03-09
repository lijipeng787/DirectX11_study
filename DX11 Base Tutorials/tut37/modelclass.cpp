


#include "modelclass.h"


ModelClass::ModelClass()
{
	vertex_buffer_=nullptr;
	m_instanceBuffer = 0;
	texture_ = nullptr;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(WCHAR* textureFilename)
{
	bool result;


	// Initialize the vertex and instance buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::Shutdown()
{
	
	ReleaseTexture();

	// Shutdown the vertex and instance buffers.
	ShutdownBuffers();

	
}


void ModelClass::Render(ID3D11DeviceContext* device_context)
{
	// Put the vertex and instance buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(device_context);

	
}


int ModelClass::GetVertexCount()
{
	return vertex_count_;
}


int ModelClass::GetInstanceCount()
{
	return m_instanceCount;
}


ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return texture_->GetTexture();
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	InstanceType* instances;
	D3D11_BUFFER_DESC vertex_buffer_desc, instanceBufferDesc;
    D3D11_SUBRESOURCE_DATA vertex_date, instanceData;
	HRESULT result;


	// Set the number of vertices in the vertex array.
	vertex_count_ = 3;

	
	auto vertices = new VertexType[vertex_count_];
	if(!vertices)
	{
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
	vertices[1].texture = XMFLOAT2(0.5f, 0.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);

	
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

	// Release the vertex array now that the vertex buffer has been created and loaded.
	delete [] vertices;
	vertices = 0;

	// Set the number of instances in the array.
	m_instanceCount = 4;

	// Create the instance array.
	instances = new InstanceType[m_instanceCount];
	if(!instances)
	{
		return false;
	}

	// Load the instance array with data.
	instances[0].position = XMFLOAT3(-1.5f, -1.5f, 5.0f);
	instances[1].position = XMFLOAT3(-1.5f,  1.5f, 5.0f);
	instances[2].position = XMFLOAT3( 1.5f, -1.5f, 5.0f);
	instances[3].position = XMFLOAT3( 1.5f,  1.5f, 5.0f);

	// Set up the description of the instance buffer.
    instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceBufferDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = 0;
    instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the instance data.
    instanceData.pSysMem = instances;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	// Create the instance buffer.
	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the instance array now that the instance buffer has been created and loaded.
	delete [] instances;
	instances = 0;

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the instance buffer.
	if(m_instanceBuffer)
	{
		m_instanceBuffer->Release();
		m_instanceBuffer = 0;
	}

	
	if(vertex_buffer_)
	{
		vertex_buffer_->Release();
		vertex_buffer_=nullptr;
	}

	
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* device_context)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];


	// Set the buffer strides.
	strides[0] = sizeof(VertexType); 
	strides[1] = sizeof(InstanceType); 

	// Set the buffer offsets.
	offsets[0] = 0;
	offsets[1] = 0;
    
	// Set the array of pointers to the vertex and instance buffers.
	bufferPointers[0] = vertex_buffer_;	
	bufferPointers[1] = m_instanceBuffer;

	
	device_context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

    
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
}


bool ModelClass::LoadTexture(WCHAR* filename)
{
	bool result;


	
	texture_ = new TextureClass();
	if(!texture_)
	{
		return false;
	}


	result = texture_->Initialize(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::ReleaseTexture()
{

	if(texture_)
	{
		texture_->Shutdown();
		delete texture_;
		texture_ = nullptr;
	}

	
}