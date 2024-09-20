#include "textclass.h"
#include "fontclass.h"
#include "fontshaderclass.h"
#include "../CommonFramework/DirectX11Device.h"

using namespace DirectX;

struct SentenceType {
	ID3D11Buffer *vertexBuffer, *indexBuffer;
	int vertexCount, indexCount, maxLength;
	float red, green, blue;
};

struct VertexType {
	XMFLOAT3 position;
	XMFLOAT2 texture;
};

bool TextClass::Initialize(HWND hwnd, int screenWidth, int screenHeight,
						   const XMMATRIX& baseViewMatrix) {

	screen_width_ = screenWidth;
	screen_height_ = screenHeight;

	base_view_matrix_ = baseViewMatrix;

	font_ = new FontClass();
	if (!font_) {
		return false;
	}

	auto result = font_->Initialize("data/fontdata.txt", L"data/font.dds");
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the font object.", L"Error", MB_OK);
		return false;
	}

	font_shader_ = new FontShaderClass();
	if (!font_shader_) {
		return false;
	}

	result = font_shader_->Initialize(hwnd);
	if (!result) {
		MessageBox(hwnd, L"Could not initialize the font shader object.", L"Error", MB_OK);
		return false;
	}

	result = InitializeSentence(&sentence_1_, 16);
	if (!result) {
		return false;
	}

	result = UpdateSentence(sentence_1_, "Hello", 100, 100, 1.0f, 1.0f, 1.0f);
	if (!result) {
		return false;
	}

	result = InitializeSentence(&sentence_2_, 16);
	if (!result) {
		return false;
	}

	result = UpdateSentence(sentence_2_, "Goodbye", 100, 200, 1.0f, 1.0f, 0.0f);
	if (!result) {
		return false;
	}

	return true;
}

void TextClass::Shutdown() {

	ReleaseSentence(&sentence_1_);

	ReleaseSentence(&sentence_2_);

	if (font_shader_) {
		font_shader_->Shutdown();
		font_shader_->~FontShaderClass();
		_aligned_free(font_shader_);
		font_shader_ = 0;
	}

	if (font_) {
		font_->Shutdown();
		delete font_;
		font_ = 0;
	}
}

bool TextClass::Render(const XMMATRIX& worldMatrix, const XMMATRIX& orthoMatrix) {

	auto result = RenderSentence(sentence_1_, worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	result = RenderSentence(sentence_2_, worldMatrix, orthoMatrix);
	if (!result) {
		return false;
	}

	return true;
}

bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength) {

	*sentence = new SentenceType;
	if (!*sentence) {
		return false;
	}

	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;

	(*sentence)->maxLength = maxLength;

	(*sentence)->vertexCount = 6 * maxLength;

	(*sentence)->indexCount = (*sentence)->vertexCount;

	auto vertices = new VertexType[(*sentence)->vertexCount];
	if (!vertices) {
		return false;
	}

	auto indices = new unsigned long[(*sentence)->indexCount];
	if (!indices) {
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	for (int i = 0; i < (*sentence)->indexCount; i++) {
		indices[i] = i;
	}

	D3D11_BUFFER_DESC vertex_buffer_desc;

	vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	vertex_buffer_desc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertex_data;

	vertex_data.pSysMem = vertices;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

	auto result = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &(*sentence)->vertexBuffer);
	if (FAILED(result)) {
		return false;
	}

	D3D11_BUFFER_DESC index_buffer_desc;

	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;
	index_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&index_buffer_desc, &indexData, &(*sentence)->indexBuffer);
	if (FAILED(result)) {
		return false;
	}

	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

bool TextClass::UpdateSentence(SentenceType* sentence, char* text, int positionX, int positionY, float red, float green, float blue) {

	sentence->red = red;
	sentence->green = green;
	sentence->blue = blue;

	auto numLetters = static_cast<int>(strlen(text));

	if (numLetters > sentence->maxLength) {
		return false;
	}

	auto vertices = new VertexType[sentence->vertexCount];
	if (!vertices) {
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	auto drawX = static_cast<float>(((screen_width_ / 2) * -1) + positionX);
	auto drawY = static_cast<float>((screen_height_ / 2) - positionY);

	font_->BuildVertexArray((void*)vertices, text, drawX, drawY);

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	auto result = device_context->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	VertexType* verticesPtr = nullptr;
	verticesPtr = (VertexType*)mappedResource.pData;

	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	device_context->Unmap(sentence->vertexBuffer, 0);

	delete[] vertices;
	vertices = 0;

	return true;
}

void TextClass::ReleaseSentence(SentenceType** sentence) {
	if (*sentence) {
		if ((*sentence)->vertexBuffer) {
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}

		if ((*sentence)->indexBuffer) {
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}

		delete *sentence;
		*sentence = 0;
	}
}

bool TextClass::RenderSentence(SentenceType* sentence, const XMMATRIX& worldMatrix, const XMMATRIX& orthoMatrix) {

	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	device_context->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

	device_context->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMFLOAT4 pixelColor = XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);

	auto result = font_shader_->Render(sentence->indexCount, worldMatrix, base_view_matrix_, orthoMatrix, font_->GetTexture(),
									   pixelColor);
	if (!result) {
		false;
	}

	return true;
}