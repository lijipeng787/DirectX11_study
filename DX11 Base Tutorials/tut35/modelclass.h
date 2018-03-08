
// Filename: modelclass.h

#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>
using namespace std;
using namespace DirectX;



// Class name: ModelClass

class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadModel(char*);
	void ReleaseModel();

private:
	ID3D11Buffer *vertex_buffer_, *index_buffer_;
	int vertex_count_, index_count_;
	ModelType* model_;
};

#endif