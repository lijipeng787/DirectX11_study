









#include <d3d11.h>
#include <DDSTextureLoader.h>
using namespace DirectX;





class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	bool Initialize(ID3D11Device*, WCHAR*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	ID3D11ShaderResourceView* texture_;
};

#endif