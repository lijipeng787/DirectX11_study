#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class TextureClass;
struct ModelType;

class ModelClass
{
public:
	ModelClass();

	ModelClass(const ModelClass&);

	~ModelClass();
public:
	bool Initialize(char*, WCHAR*);

	void Shutdown();
	
	void Render();

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

	void SetPosition(float, float, float);
	
	void GetPosition(float&, float&, float&);
private:
	bool InitializeBuffers();

	void ShutdownBuffers();
	
	void RenderBuffers();

	bool LoadTexture(WCHAR*);
	
	void ReleaseTexture();

	bool LoadModel(char*);
	
	void ReleaseModel();
private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;

	int vertex_count_, index_count_;
	
	TextureClass* texture_;
	
	ModelType* model_;
	
	float position_x_, position_y_, position_z_;
};
