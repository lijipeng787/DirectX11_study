









#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;





#include "textureclass.h"





class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

	struct InstanceType
	{
		XMFLOAT3 position;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetVertexCount();
	int GetInstanceCount();
	ID3D11ShaderResourceView* GetTexture();
	
private:
	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

private:
	ID3D11Buffer* vertex_buffer_;
	ID3D11Buffer* m_instanceBuffer;
	int vertex_count_;
	int m_instanceCount;
	TextureClass* texture_;
};

#endif