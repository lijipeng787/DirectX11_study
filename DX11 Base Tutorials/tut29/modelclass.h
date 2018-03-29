#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

struct VertexType;
struct ModelType;

class TextureClass;

class ModelClass {
public:
	ModelClass() {}

	ModelClass(const ModelClass&) = delete;

	~ModelClass() {}
public:
	bool Initialize(WCHAR*, char*);

	void Shutdown();

	void Render();

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();
private:
	bool InitializeBuffers();

	void ShutdownBuffers();

	void RenderBuffers();

	bool LoadTexture(WCHAR*);

	void ReleaseTexture();

	bool LoadModel(char*);

	void ReleaseModel();
private:
	ID3D11Buffer * vertex_buffer_ = nullptr, *index_buffer_ = nullptr;

	int vertex_count_ = 0, index_count_ = 0;

	TextureClass* texture_ = nullptr;

	ModelType* model_ = nullptr;
};