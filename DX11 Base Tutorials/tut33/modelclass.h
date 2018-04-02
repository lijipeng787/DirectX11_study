#pragma once

#include <d3d11.h>

struct ModelType;
class TextureClass;

class ModelClass
{
public:
	ModelClass() {}

	ModelClass(const ModelClass& rhs)=delete;
	
	~ModelClass() {}
public:
	bool Initialize(char*, WCHAR*, WCHAR*, WCHAR*);
	
	void Shutdown();
	
	void Render();

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture1();
	
	ID3D11ShaderResourceView* GetTexture2();
	
	ID3D11ShaderResourceView* GetTexture3();
private:
	bool InitializeBuffers();

	void ShutdownBuffers();
	
	void RenderBuffers();

	bool LoadTextures(WCHAR*, WCHAR*, WCHAR*);
	
	void ReleaseTextures();

	bool LoadModel(char*);
	
	void ReleaseModel();
private:
	ID3D11Buffer *vertex_buffer_=nullptr, *index_buffer_ = nullptr;

	int vertex_count_, index_count_;
	
	TextureClass *texture_1_ = nullptr, *texture_2_ = nullptr, *texture_3_ = nullptr;
	
	ModelType* model_ = nullptr;
};