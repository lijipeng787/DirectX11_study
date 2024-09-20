#include "textclass.h"
#include "../CommonFramework/DirectX11Device.h"

#include "fontclass.h"
#include "fontshaderclass.h"

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

bool TextClass::Initialize(HWND hwnd, int screenWidth, int screenHeight, const XMMATRIX& baseViewMatrix) {

	screen_width_ = screenWidth;
	screen_height_ = screenHeight;

	XMStoreFloat4x4(&base_view_matrix_, baseViewMatrix);

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

	result = InitializeSentence(&sentence_1_, 32);
	if (!result) {
		return false;
	}

	result = UpdateSentence(sentence_1_, "Render Count: ", 20, 20, 1.0f, 1.0f, 1.0f);
	if (!result) {
		return false;
	}

	return true;
}

void TextClass::Shutdown() {

	ReleaseSentence(&sentence_1_);

	if (font_shader_) {
		font_shader_->Shutdown();
		delete font_shader_;
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

	auto numLetters = (int)strlen(text);

	if (numLetters > sentence->maxLength) {
		return false;
	}

	auto vertices = new VertexType[sentence->vertexCount];
	if (!vertices) {
		return false;
	}

	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	auto drawX = (float)(((screen_width_ / 2) * -1) + positionX);
	auto drawY = (float)((screen_height_ / 2) - positionY);

	font_->BuildVertexArray((void*)vertices, text, drawX, drawY);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto device_context = DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

	auto result = device_context->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		return false;
	}

	auto verticesPtr = (VertexType*)mappedResource.pData;

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

	auto pixelColor = XMFLOAT4(sentence->red, sentence->green, sentence->blue, 1.0f);

	XMMATRIX base_view = XMLoadFloat4x4(&base_view_matrix_);

	auto result = font_shader_->Render(sentence->indexCount, worldMatrix, base_view, orthoMatrix, font_->GetTexture(),
									   pixelColor);
	if (!result) {
		false;
	}

	return true;
}

bool TextClass::SetRenderCount(int count) {

	char tempString[32] = {};
	char countString[32] = {};

	// Convert the count integer to string format.
	_itoa_s(count, tempString, 10);

	// Setup the render count string.
	strcpy_s(countString, "Render Count: ");
	strcat_s(countString, tempString);

	// Update the sentence vertex buffer with the new string information.
	auto result = UpdateSentence(sentence_1_, countString, 20, 20, 1.0f, 1.0f, 1.0f);
	if (!result) {
		return false;
	}

	return true;
}