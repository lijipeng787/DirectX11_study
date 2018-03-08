
// Filename: textclass.h

#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_

/////////
// MY CLASS INCLUDES //
/////////
#include "fontclass.h"
#include "fontshaderclass.h"



// Class name: TextClass

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

	bool Initialize(ID3D11Device*, HWND, int, int, const XMMATRIX& );
	void Shutdown();
	bool Render(const XMMATRIX&, const XMMATRIX& );

	bool SetIntersection(bool, ID3D11DeviceContext*);

private:
	bool InitializeSentence(SentenceType**, int, ID3D11Device*);
	bool UpdateSentence(SentenceType*, char*, int, int, float, float, float, ID3D11DeviceContext*);
	void ReleaseSentence(SentenceType**);
	bool RenderSentence(SentenceType*, const XMMATRIX&, const XMMATRIX& );

private:
	FontClass* m_Font;
	FontShaderClass* m_FontShader;
	int m_screenWidth, m_screenHeight;
	XMMATRIX m_baseViewMatrix;
	SentenceType* m_sentence1;
};

#endif