


#include "modelclass.h"


ModelClass::ModelClass()
{
	vertex_buffer_ = 0;
	index_buffer_ = 0;
	texture_ = 0;
	model_ = 0;
}


ModelClass::ModelClass(const ModelClass& other)
{
}


ModelClass::~ModelClass()
{
}


bool ModelClass::Initialize(ID3D11Device* device, char* modelFilename, WCHAR* textureFilename)
{
	bool result;


	
	result = LoadModel(modelFilename);
	if(!result)
	{
		return false;
	}

	
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

	
	ShutdownBuffers();

	
	ReleaseModel();

	
}


void ModelClass::Render(ID3D11DeviceContext* device_context)
{
	
	RenderBuffers(device_context);

	
}


int ModelClass::GetIndexCount()
{
	return index_count_;
}


ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return texture_->GetTexture();
}


bool ModelClass::InitializeBuffers(ID3D11Device* device)
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

    
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
}


bool ModelClass::LoadTexture(ID3D11Device* device, WCHAR* filename)
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
		texture_ = 0;
	}

	
}


bool ModelClass::LoadModel(char* filename)
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


void ModelClass::ReleaseModel()
{
	if(model_)
	{
		delete [] model_;
		model_ = 0;
	}

	
}