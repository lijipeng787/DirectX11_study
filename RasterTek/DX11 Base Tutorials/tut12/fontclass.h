#ifndef _FONTCLASS_H_
#define _FONTCLASS_H_

#include <DirectXMath.h>
#include <fstream>
#include <memory>
#include "textureclass.h"

class Font{

private:
	struct FontType{
		float left_;
		float right_;
		int size_;
	};

	struct VertexType{
		DirectX::XMFLOAT3 position_;
	    DirectX::XMFLOAT2 texture_;
	};

public:
	Font() {}

	explicit Font(const Font& copy) {}
	
	~Font() { if (nullptr != m_Font) { delete m_Font; } }

public:
	bool Initialize(WCHAR* font_filename, WCHAR* texture_filename);
	
	void BuildVertexArray(void* vertices, WCHAR* sentence, float drawX, float drawY);

	bool LoadFontData(WCHAR* font_name);
	
	bool LoadTexture(WCHAR* texture_name);

	DescriptorHeapPtr GetTexture()const { return m_texture->get_texture(); }
	
private:
	FontType *m_Font = nullptr;

	std::shared_ptr<Texture> m_texture = nullptr;
};

#endif