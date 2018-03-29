


#include "textclass.h"


TextClass::TextClass()
{
	font_ = 0;
	font_shader_ = 0;
	sentence_1_ = 0;
}


TextClass::TextClass(const TextClass& other)
{
}


TextClass::~TextClass()
{
}


bool TextClass::Initialize(HWND hwnd, int screenWidth, int screenHeight, 
						   const XMMATRIX& baseViewMatrix )
{
	bool result;



	screen_width_ = screenWidth;
	screen_height_ = screenHeight;


	base_view_matrix_ = baseViewMatrix;


	font_ = new FontClass();
	if(!font_)
	{
		return false;
	}


	result = font_->Initialize(device, "../../tut47/data/fontdata.txt", L"../../tut47/data/font.dds");
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}


	font_shader_ = new FontShaderClass();
	if(!font_shader_)
	{
		return false;
	}


	result = font_shader_->Initialize(device, hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}


	result = InitializeSentence(&sentence_1_, 32, device);
	if(!result)
	{
		return false;
	}


	result = UpdateSentence(sentence_1_, "Intersection: No", 20, 20, 1.0f, 0.0f, 0.0f, device_context);
	if(!result)
	{
		return false;
	}

	return true;
}


void TextClass::Shutdown()
{

	ReleaseSentence(&sentence_1_);


	if(font_shader_)
	{
		font_shader_->Shutdown();
		delete font_shader_;
		font_shader_ = 0;
	}


	if(font_)
	{
		font_->Shutdown();
		delete font_;
		font_ = 0;
	}

	
}


bool TextClass::Render(const XMMATRIX& worldMatrix, const XMMATRIX& orthoMatrix )
{
	bool result;



	result = RenderSentence(sentence_1_, worldMatrix, orthoMatrix);
	if(!result)
	{
		return false;
	}

	return true;
}


bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength, ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
    D3D11_SUBRESOURCE_DATA vertex_data, indexData;
	HRESULT result;
	int i;



	*sentence = new SentenceType;
	if(!*sentence)
	{
		return false;
	}


	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;


	(*sentence)->maxLength = maxLength;

	
	(*sentence)->vertexCount = 6 * maxLength;


	(*sentence)->indexCount = (*sentence)->vertexCount;

	
	vertices = new VertexType[(*sentence)->vertexCount];
	if(!vertices)
	{
		return false;
	}

	
	indices = new unsigned long[(*sentence)->indexCount];
	if(!indices)
	{
		return false;
	}

	
	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));


	for(i=0; i<(*sentence)->indexCount; i++)
	{
		indices[i] = i;
	}


    vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_buffer_desc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	
    vertex_data.pSysMem = vertices;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;


    result = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &(*sentence)->vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	
    index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    index_buffer_desc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_buffer_desc.CPUAccessFlags = 0;
    index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	
	result = device->CreateBuffer(&index_buffer_desc, &indexData, &(*sentence)->indexBuffer);
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


bool TextClass::UpdateSentence(SentenceType* sentence, char* text, int positionX, int positionY, float red, float green, float blue,
							   ID3D11DeviceContext* device_context)
{
	int numLetters;
	VertexType* vertices;
	float drawX, drawY;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;



	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;


	numLetters = (int)strlen(text);


	if(numLetters > sentence->maxLength)
	{
		return false;
	}

	
	vertices = new VertexType[sentence->vertexCount];
	if(!vertices)
	{
		return false;
	}

	
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));


	drawX = (float)(((screen_width_ / 2) * -1) + positionX);
	drawY = (float)((screen_height_ / 2) - positionY);


	font_->BuildVertexArray((void*)vertices, text, drawX, drawY);

	
	result = device_context->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	
	verticesPtr = (VertexType*)mappedResource.pData;

	
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	
	device_context->Unmap(sentence->vertexBuffer, 0);

	
	delete [] vertices;
	vertices = 0;

	return true;
}


void TextClass::ReleaseSentence(SentenceType** sentence)
{
	if(*sentence)
	{

		if((*sentence)->vertexBuffer)
		{
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}


		if((*sentence)->indexBuffer)
		{
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}


		delete *sentence;
		*sentence = 0;
	}

	
}


bool TextClass::RenderSentence(SentenceType* sentence, const XMMATRIX& worldMatrix, 
							   const XMMATRIX& orthoMatrix )
{
	unsigned int stride, offset;
	XMFLOAT4 pixelColor;
	bool result;


	
    stride = sizeof(VertexType); 
	offset = 0;

	
	device_context->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

    
	device_context->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    
	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	pixelColor = XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);


	result = font_shader_->Render(sentence->indexCount, worldMatrix, base_view_matrix_, orthoMatrix, font_->GetTexture(), 
								  pixelColor);
	if(!result)
	{
		false;
	}

	return true;
}


bool TextClass::SetIntersection(bool intersection, ID3D11DeviceContext* device_context)
{
	char intersectionString[32];
	bool result;


	if(intersection)
	{
		strcpy_s(intersectionString, "Intersection: Yes");
		result = UpdateSentence(sentence_1_, intersectionString, 20, 20, 0.0f, 1.0f, 0.0f, device_context);
	}
	else
	{
		strcpy_s(intersectionString, "Intersection: No");
		result = UpdateSentence(sentence_1_, intersectionString, 20, 20, 1.0f, 0.0f, 0.0f, device_context);
	}

	return result;
}