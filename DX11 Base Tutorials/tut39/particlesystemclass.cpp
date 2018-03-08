
// Filename: particlesystemclass.cpp

#include "particlesystemclass.h"


ParticleSystemClass::ParticleSystemClass()
{
	texture_ = 0;
	m_particleList = 0;
	m_vertices = 0;
	vertex_buffer_ = 0;
	index_buffer_ = 0;
}


ParticleSystemClass::ParticleSystemClass(const ParticleSystemClass& other)
{
}


ParticleSystemClass::~ParticleSystemClass()
{
}


bool ParticleSystemClass::Initialize(ID3D11Device* device, WCHAR* textureFilename)
{
	bool result;


	// Load the texture that is used for the particles.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	// Initialize the particle system.
	result = InitializeParticleSystem();
	if(!result)
	{
		return false;
	}

	// Create the buffers that will be used to render the particles with.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	return true;
}


void ParticleSystemClass::Shutdown()
{
	// Release the buffers.
	ShutdownBuffers();

	// Release the particle system.
	ShutdownParticleSystem();

	// Release the texture used for the particles.
	ReleaseTexture();

	
}


bool ParticleSystemClass::Frame(float frameTime, ID3D11DeviceContext* device_context)
{
	bool result;


	// Release old particles.
	KillParticles();

	// Emit new particles.
	EmitParticles(frameTime);
	
	// Update the position of the particles.
	UpdateParticles(frameTime);

	// Update the dynamic vertex buffer with the new position of each particle.
	result = UpdateBuffers(device_context);
	if(!result)
	{
		return false;
	}

	return true;
}


void ParticleSystemClass::Render(ID3D11DeviceContext* device_context)
{
	
	RenderBuffers(device_context);

	
}


ID3D11ShaderResourceView* ParticleSystemClass::GetTexture()
{
	return texture_->GetTexture();
}


int ParticleSystemClass::GetIndexCount()
{
	return index_count_;
}


bool ParticleSystemClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
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


void ParticleSystemClass::ReleaseTexture()
{

	if(texture_)
	{
		texture_->Shutdown();
		delete texture_;
		texture_ = 0;
	}

	
}


bool ParticleSystemClass::InitializeParticleSystem()
{
	int i;


	// Set the random deviation of where the particles can be located when emitted.
	m_particleDeviationX = 0.5f;
	m_particleDeviationY = 0.1f;
	m_particleDeviationZ = 2.0f;

	// Set the speed and speed variation of particles.
	m_particleVelocity = 1.0f;
	m_particleVelocityVariation = 0.2f;

	// Set the physical size of the particles.
	m_particleSize = 0.2f;

	// Set the number of particles to emit per second.
	m_particlesPerSecond = 250.0f;

	// Set the maximum number of particles allowed in the particle system.
	m_maxParticles = 5000;

	// Create the particle list.
	m_particleList = new ParticleType[m_maxParticles];
	if(!m_particleList)
	{
		return false;
	}

	// Initialize the particle list.
	for(i=0; i<m_maxParticles; i++)
	{
		m_particleList[i].active = false;
	}

	// Initialize the current particle count to zero since none are emitted yet.
	m_currentParticleCount = 0;

	// Clear the initial accumulated time for the particle per second emission rate.
	m_accumulatedTime = 0.0f;

	return true;
}


void ParticleSystemClass::ShutdownParticleSystem()
{
	// Release the particle list.
	if(m_particleList)
	{
		delete [] m_particleList;
		m_particleList = 0;
	}

	
}


bool ParticleSystemClass::InitializeBuffers(ID3D11Device* device)
{
	unsigned long* indices;
	int i;
	D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
    D3D11_SUBRESOURCE_DATA vertex_date, indexData;
	HRESULT result;


	// Set the maximum number of vertices in the vertex array.
	vertex_count_ = m_maxParticles * 6;

	// Set the maximum number of indices in the index array.
	index_count_ = vertex_count_;

	// Create the vertex array for the particles that will be rendered.
	m_vertices = new VertexType[vertex_count_];
	if(!m_vertices)
	{
		return false;
	}

	
	indices = new unsigned long[index_count_];
	if(!indices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexType) * vertex_count_));

	// Initialize the index array.
	for(i=0; i<index_count_; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	
    vertex_date.pSysMem = m_vertices;
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

	// Release the index array since it is no longer needed.
	delete [] indices;
	indices = 0;

	return true;
}


void ParticleSystemClass::ShutdownBuffers()
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


void ParticleSystemClass::EmitParticles(float frameTime)
{
	bool emitParticle, found;
	float positionX, positionY, positionZ, velocity, red, green, blue;
	int index, i, j;


	// Increment the frame time.
	m_accumulatedTime += frameTime;

	// Set emit particle to false for now.
	emitParticle = false;
	
	// Check if it is time to emit a new particle or not.
	if(m_accumulatedTime > (1000.0f / m_particlesPerSecond))
	{
		m_accumulatedTime = 0.0f;
		emitParticle = true;
	}

	// If there are particles to emit then emit one per frame.
	if((emitParticle == true) && (m_currentParticleCount < (m_maxParticles - 1)))
	{
		m_currentParticleCount++;

		// Now generate the randomized particle properties.
		positionX = (((float)rand()-(float)rand())/RAND_MAX) * m_particleDeviationX;
		positionY = (((float)rand()-(float)rand())/RAND_MAX) * m_particleDeviationY;
		positionZ = (((float)rand()-(float)rand())/RAND_MAX) * m_particleDeviationZ;

		velocity = m_particleVelocity + (((float)rand()-(float)rand())/RAND_MAX) * m_particleVelocityVariation;

		red   = (((float)rand()-(float)rand())/RAND_MAX) + 0.5f;
		green = (((float)rand()-(float)rand())/RAND_MAX) + 0.5f;
		blue  = (((float)rand()-(float)rand())/RAND_MAX) + 0.5f;

		// Now since the particles need to be rendered from back to front for blending we have to sort the particle array.
		// We will sort using Z depth so we need to find where in the list the particle should be inserted.
		index = 0;
		found = false;
		while(!found)
		{
			if((m_particleList[index].active == false) || (m_particleList[index].positionZ < positionZ))
			{
				found = true;
			}
			else
			{
				index++;
			}
		}

		// Now that we know the location to insert into we need to copy the array over by one position from the index to make room for the new particle.
		i = m_currentParticleCount;
		j = i - 1;

		while(i != index)
		{
			m_particleList[i].positionX = m_particleList[j].positionX;
			m_particleList[i].positionY = m_particleList[j].positionY;
			m_particleList[i].positionZ = m_particleList[j].positionZ;
			m_particleList[i].red       = m_particleList[j].red;
			m_particleList[i].green     = m_particleList[j].green;
			m_particleList[i].blue      = m_particleList[j].blue;
			m_particleList[i].velocity  = m_particleList[j].velocity;
			m_particleList[i].active    = m_particleList[j].active;
			i--;
			j--;
		}

		// Now insert it into the particle array in the correct depth order.
		m_particleList[index].positionX = positionX;
		m_particleList[index].positionY = positionY;
		m_particleList[index].positionZ = positionZ;
		m_particleList[index].red       = red;
		m_particleList[index].green     = green;
		m_particleList[index].blue      = blue;
		m_particleList[index].velocity  = velocity;
		m_particleList[index].active    = true;
	}

	
}


void ParticleSystemClass::UpdateParticles(float frameTime)
{
	int i;


	// Each frame we update all the particles by making them move downwards using their position, velocity, and the frame time.
	for(i=0; i<m_currentParticleCount; i++)
	{
		m_particleList[i].positionY = m_particleList[i].positionY - (m_particleList[i].velocity * frameTime * 0.001f);
	}

	
}


void ParticleSystemClass::KillParticles()
{
	int i, j;


	// Kill all the particles that have gone below a certain height range.
	for(i=0; i<m_maxParticles; i++)
	{
		if((m_particleList[i].active == true) && (m_particleList[i].positionY < -3.0f))
		{
			m_particleList[i].active = false;
			m_currentParticleCount--;

			// Now shift all the live particles back up the array to erase the destroyed particle and keep the array sorted correctly.
			for(j=i; j<m_maxParticles-1; j++)
			{
				m_particleList[j].positionX = m_particleList[j+1].positionX;
				m_particleList[j].positionY = m_particleList[j+1].positionY;
				m_particleList[j].positionZ = m_particleList[j+1].positionZ;
				m_particleList[j].red       = m_particleList[j+1].red;
				m_particleList[j].green     = m_particleList[j+1].green;
				m_particleList[j].blue      = m_particleList[j+1].blue;
				m_particleList[j].velocity  = m_particleList[j+1].velocity;
				m_particleList[j].active    = m_particleList[j+1].active;
			}
		}
	}

	
}


bool ParticleSystemClass::UpdateBuffers(ID3D11DeviceContext* device_context)
{
	int index, i;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;


	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(VertexType) * vertex_count_));

	// Now build the vertex array from the particle list array.  Each particle is a quad made out of two triangles.
	index = 0;

	for(i=0; i<m_currentParticleCount; i++)
	{
		// Bottom left.
		m_vertices[index].position = XMFLOAT3(m_particleList[i].positionX - m_particleSize, m_particleList[i].positionY - m_particleSize, m_particleList[i].positionZ);
		m_vertices[index].texture = XMFLOAT2(0.0f, 1.0f);
		m_vertices[index].color = XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
		index++;

		// Top left.
		m_vertices[index].position = XMFLOAT3(m_particleList[i].positionX - m_particleSize, m_particleList[i].positionY + m_particleSize, m_particleList[i].positionZ);
		m_vertices[index].texture = XMFLOAT2(0.0f, 0.0f);
		m_vertices[index].color = XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
		index++;

		// Bottom right.
		m_vertices[index].position = XMFLOAT3(m_particleList[i].positionX + m_particleSize, m_particleList[i].positionY - m_particleSize, m_particleList[i].positionZ);
		m_vertices[index].texture = XMFLOAT2(1.0f, 1.0f);
		m_vertices[index].color = XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
		index++;

		// Bottom right.
		m_vertices[index].position = XMFLOAT3(m_particleList[i].positionX + m_particleSize, m_particleList[i].positionY - m_particleSize, m_particleList[i].positionZ);
		m_vertices[index].texture = XMFLOAT2(1.0f, 1.0f);
		m_vertices[index].color = XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
		index++;

		// Top left.
		m_vertices[index].position = XMFLOAT3(m_particleList[i].positionX - m_particleSize, m_particleList[i].positionY + m_particleSize, m_particleList[i].positionZ);
		m_vertices[index].texture = XMFLOAT2(0.0f, 0.0f);
		m_vertices[index].color = XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
		index++;

		// Top right.
		m_vertices[index].position = XMFLOAT3(m_particleList[i].positionX + m_particleSize, m_particleList[i].positionY + m_particleSize, m_particleList[i].positionZ);
		m_vertices[index].texture = XMFLOAT2(1.0f, 0.0f);
		m_vertices[index].color = XMFLOAT4(m_particleList[i].red, m_particleList[i].green, m_particleList[i].blue, 1.0f);
		index++;
	}
	
	// Lock the vertex buffer.
	result = device_context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(VertexType) * vertex_count_));

	// Unlock the vertex buffer.
	device_context->Unmap(vertex_buffer_, 0);

	return true;
}


void ParticleSystemClass::RenderBuffers(ID3D11DeviceContext* device_context)
{
	unsigned int stride;
	unsigned int offset;


	
    stride = sizeof(VertexType); 
	offset = 0;
    
	
	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

    
    device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer.
    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
}