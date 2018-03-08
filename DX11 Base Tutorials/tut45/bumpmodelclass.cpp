
// Filename: bumpmodelclass.cpp

#include "bumpmodelclass.h"


BumpModelClass::BumpModelClass()
{
	vertex_buffer_ = 0;
	index_buffer_ = 0;
	model_ = 0;
	m_ColorTexture = 0;
	m_NormalMapTexture = 0;
}


BumpModelClass::BumpModelClass(const BumpModelClass& other)
{
}


BumpModelClass::~BumpModelClass()
{
}


bool BumpModelClass::Initialize(ID3D11Device* device, char* modelFilename, WCHAR* textureFilename1, WCHAR* textureFilename2)
{
	bool result;


	// Load in the model data,
	result = LoadModel(modelFilename);
	if(!result)
	{
		return false;
	}

	// Calculate the tangent and binormal vectors for the model.
	CalculateModelVectors();

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	// Load the textures for this model.
	result = LoadTextures(device, textureFilename1, textureFilename2);
	if(!result)
	{
		return false;
	}

	return true;
}


void BumpModelClass::Shutdown()
{
	// Release the model textures.
	ReleaseTextures();

	
	ShutdownBuffers();

	
	ReleaseModel();

	
}


void BumpModelClass::Render(ID3D11DeviceContext* device_context)
{
	
	RenderBuffers(device_context);

	
}


int BumpModelClass::GetIndexCount()
{
	return index_count_;
}


ID3D11ShaderResourceView* BumpModelClass::GetColorTexture()
{
	return m_ColorTexture->GetTexture();
}


ID3D11ShaderResourceView* BumpModelClass::GetNormalMapTexture()
{
	return m_NormalMapTexture->GetTexture();
}


bool BumpModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
	D3D11_SUBRESOURCE_DATA vertex_date, indexData;
	HRESULT result;
	int i;


	
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

	
	for(i=0; i<vertex_count_; i++)
	{
		vertices[i].position = XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
		vertices[i].texture = XMFLOAT2(model_[i].tu, model_[i].tv);
		vertices[i].normal = XMFLOAT3(model_[i].nx, model_[i].ny, model_[i].nz);
		vertices[i].tangent = XMFLOAT3(model_[i].tx, model_[i].ty, model_[i].tz);
		vertices[i].binormal = XMFLOAT3(model_[i].bx, model_[i].by, model_[i].bz);

		indices[i] = i;
	}

	
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


void BumpModelClass::ShutdownBuffers()
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


void BumpModelClass::RenderBuffers(ID3D11DeviceContext* device_context)
{
	unsigned int stride;
	unsigned int offset;


	
	stride = sizeof(VertexType); 
	offset = 0;
    
	
	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

    
	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

    
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
}


bool BumpModelClass::LoadTextures(ID3D11Device* device, WCHAR* filename1, WCHAR* filename2)
{
	bool result;


	// Create the color texture object.
	m_ColorTexture = new TextureClass();
	if(!m_ColorTexture)
	{
		return false;
	}

	// Initialize the color texture object.
	result = m_ColorTexture->Initialize(device, filename1);
	if(!result)
	{
		return false;
	}

	// Create the normal map texture object.
	m_NormalMapTexture = new TextureClass();
	if(!m_NormalMapTexture)
	{
		return false;
	}

	// Initialize the normal map texture object.
	result = m_NormalMapTexture->Initialize(device, filename2);
	if(!result)
	{
		return false;
	}

	return true;
}


void BumpModelClass::ReleaseTextures()
{
	// Release the texture objects.
	if(m_ColorTexture)
	{
		m_ColorTexture->Shutdown();
		delete m_ColorTexture;
		m_ColorTexture = 0;
	}

	if(m_NormalMapTexture)
	{
		m_NormalMapTexture->Shutdown();
		delete m_NormalMapTexture;
		m_NormalMapTexture = 0;
	}

	
}


bool BumpModelClass::LoadModel(char* filename)
{
	ifstream fin;
	char input;
	int i;


	
	fin.open(filename);
	if(fin.fail())
	{
		return false;
	}

	
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

	
	for(i=0; i<vertex_count_; i++)
	{
		fin >> model_[i].x >> model_[i].y >> model_[i].z;
		fin >> model_[i].tu >> model_[i].tv;
		fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;
	}

	
	fin.close();

	return true;
}


void BumpModelClass::ReleaseModel()
{
	if(model_)
	{
		delete [] model_;
		model_ = 0;
	}

	
}


void BumpModelClass::CalculateModelVectors()
{
	int faceCount, i, index;
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;


	// Calculate the number of faces in the model.
	faceCount = vertex_count_ / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for(i=0; i<faceCount; i++)
	{
		// Get the three vertices for this face from the model.
		vertex1.x = model_[index].x;
		vertex1.y = model_[index].y;
		vertex1.z = model_[index].z;
		vertex1.tu = model_[index].tu;
		vertex1.tv = model_[index].tv;
		vertex1.nx = model_[index].nx;
		vertex1.ny = model_[index].ny;
		vertex1.nz = model_[index].nz;
		index++;

		vertex2.x = model_[index].x;
		vertex2.y = model_[index].y;
		vertex2.z = model_[index].z;
		vertex2.tu = model_[index].tu;
		vertex2.tv = model_[index].tv;
		vertex2.nx = model_[index].nx;
		vertex2.ny = model_[index].ny;
		vertex2.nz = model_[index].nz;
		index++;

		vertex3.x = model_[index].x;
		vertex3.y = model_[index].y;
		vertex3.z = model_[index].z;
		vertex3.tu = model_[index].tu;
		vertex3.tv = model_[index].tv;
		vertex3.nx = model_[index].nx;
		vertex3.ny = model_[index].ny;
		vertex3.nz = model_[index].nz;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the normal, tangent, and binormal for this face back in the model structure.
		model_[index-1].tx = tangent.x;
		model_[index-1].ty = tangent.y;
		model_[index-1].tz = tangent.z;
		model_[index-1].bx = binormal.x;
		model_[index-1].by = binormal.y;
		model_[index-1].bz = binormal.z;

		model_[index-2].tx = tangent.x;
		model_[index-2].ty = tangent.y;
		model_[index-2].tz = tangent.z;
		model_[index-2].bx = binormal.x;
		model_[index-2].by = binormal.y;
		model_[index-2].bz = binormal.z;

		model_[index-3].tx = tangent.x;
		model_[index-3].ty = tangent.y;
		model_[index-3].tz = tangent.z;
		model_[index-3].bx = binormal.x;
		model_[index-3].by = binormal.y;
		model_[index-3].bz = binormal.z;
	}

	
}


void BumpModelClass::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3,
										  VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));
			
	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));
			
	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	
}