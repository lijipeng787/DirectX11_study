








#include "fontclass.h"
#include "fontshaderclass.h"





class TextClass
{
private:
	struct SentenceType
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		int vertexCount, indexCount, maxLength;
		float red, green, blue;
	};

	struct VertexType
	{
		XMFLOAT3 position;
	    XMFLOAT2 texture;
	};

public:
	TextClass();
	TextClass(const TextClass&);
	~TextClass();

	bool Initialize(HWND, int, int, const XMMATRIX& );
	void Shutdown();
	bool Render(const XMMATRIX&, const XMMATRIX& );

	bool SetRenderCount(int, ID3D11DeviceContext*);

private:
	bool InitializeSentence(SentenceType**, int, ID3D11Device*);
	bool UpdateSentence(SentenceType*, char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(SentenceType**);
	bool RenderSentence(SentenceType*, const XMMATRIX&, const XMMATRIX& );

private:
	FontClass* font_;
	FontShaderClass* font_shader_;
	int screen_width_, screen_height_;
	XMMATRIX base_view_matrix_;
	SentenceType* sentence_1_;
};

#endif