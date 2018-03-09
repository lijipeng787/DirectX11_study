
// Filename: treeclass.h

#ifndef _TREECLASS_H_
#define _TREEMODELCLASS_H_





#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>
using namespace std;





#include "textureclass.h"



// Class name: TreeClass

class TreeClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	TreeClass();
	TreeClass(const TreeClass&);
	~TreeClass();

	bool Initialize(char*, WCHAR*, char*, WCHAR*, float);
	void Shutdown();

	void RenderTrunk(ID3D11DeviceContext*);
	void RenderLeaves(ID3D11DeviceContext*);

	int GetTrunkIndexCount();
	int GetLeafIndexCount();
	
	ID3D11ShaderResourceView* GetTrunkTexture();
	ID3D11ShaderResourceView* GetLeafTexture();
	
	void SetPosition(float, float, float);
	void GetPosition(float&, float&, float&);

private:
	bool InitializeTrunkBuffers(float);
	bool InitializeLeafBuffers(float);

	void ShutdownBuffers();

	void RenderTrunkBuffers(ID3D11DeviceContext*);
	void RenderLeafBuffers(ID3D11DeviceContext*);

	bool LoadTextures(WCHAR*, WCHAR*);
	void ReleaseTextures();

	bool LoadModel(char*);
	void ReleaseModel();

private:
	ID3D11Buffer *m_trunkVertexBuffer, *m_trunkIndexBuffer, *m_leafVertexBuffer, *m_leafIndexBuffer;
	int vertex_count_, index_count_, m_trunkIndexCount, m_leafIndexCount;
	TextureClass *m_TrunkTexture, *m_LeafTexture;
	ModelType* model_;
	float m_positionX, m_positionY, m_positionZ;
};

#endif