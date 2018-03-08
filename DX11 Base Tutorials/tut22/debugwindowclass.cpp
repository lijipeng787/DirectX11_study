
// Filename: debugwindowclass.cpp

#include "debugwindowclass.h"


DebugWindowClass::DebugWindowClass()
{
	vertex_buffer_ = 0;
	index_buffer_ = 0;
}


DebugWindowClass::DebugWindowClass(const DebugWindowClass& other)
{
}


DebugWindowClass::~DebugWindowClass()
{
}


bool DebugWindowClass::Initialize(ID3D11Device* device, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight)
{
	bool result;


	// Store the screen size.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the size in pixels that this bitmap should be rendered at.
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;

	// Initialize the previous rendering position to negative one.
	m_previousPosX = -1;
	m_previousPosY = -1;

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}


void DebugWindowClass::Shutdown()
{
	
	ShutdownBuffers();

	
}


bool DebugWindowClass::Render(ID3D11DeviceContext* device_context, int positionX, int positionY)
{
	bool result;


	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	result = UpdateBuffers(device_context, positionX, positionY);
	if(!result)
	{
		return false;
	}

	
	RenderBuffers(device_context);

	return true;
}


int DebugWindowClass::GetIndexCount()
{
	return index_count_;
}


bool DebugWindowClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
	D3D11_SUBRESOURCE_DATA vertex_date, indexData;
	HRESULT result;
	int i;


	// Set the number of vertices in the vertex array.
	vertex_count_ = 6;

	// Set the number of indices in the index array.
	index_count_ = vertex_count_;

	
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

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * vertex_count_));

	
	for(i=0; i<index_count_; i++)
	{
		indices[i] = i;
	}

	
    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
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


void DebugWindowClass::ShutdownBuffers()
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


bool DebugWindowClass::UpdateBuffers(ID3D11DeviceContext* device_context, int positionX, int positionY)
{
	float left, right, top, bottom;
	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT result;


	// If the position we are rendering this bitmap to has not changed then don't update the vertex buffer since it
	// currently has the correct parameters.
	if((positionX == m_previousPosX) && (positionY == m_previousPosY))
	{
		return true;
	}
	
	// If it has changed then update the position it is being rendered to.
	m_previousPosX = positionX;
	m_previousPosY = positionY;

	// Calculate the screen coordinates of the left side of the bitmap.
	left = (float)((m_screenWidth / 2) * -1) + (float)positionX;

	// Calculate the screen coordinates of the right side of the bitmap.
	right = left + (float)m_bitmapWidth;

	// Calculate the screen coordinates of the top of the bitmap.
	top = (float)(m_screenHeight / 2) - (float)positionY;

	// Calculate the screen coordinates of the bottom of the bitmap.
	bottom = top - (float)m_bitmapHeight;

	
	vertices = new VertexType[vertex_count_];
	if(!vertices)
	{
		return false;
	}

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	// Lock the vertex buffer so it can be written to.
	result = device_context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * vertex_count_));

	// Unlock the vertex buffer.
	device_context->Unmap(vertex_buffer_, 0);

	// Release the vertex array as it is no longer needed.
	delete [] vertices;
	vertices = 0;

	return true;
}


void DebugWindowClass::RenderBuffers(ID3D11DeviceContext* device_context)
{
	unsigned int stride;
	unsigned int offset;


	
	stride = sizeof(VertexType); 
	offset = 0;
    
	
	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

    
	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

    
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
}