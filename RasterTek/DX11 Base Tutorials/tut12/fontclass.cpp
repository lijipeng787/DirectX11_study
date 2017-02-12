#include "fontclass.h"

bool Font::Initialize(WCHAR* font_filename, WCHAR* texture_filename){

	// Load in the text file containing the font data.
	if(CHECK(LoadFontData(font_filename))){
		return false;
	}

	// Load the texture that has the font characters on it.
	if(CHECK(LoadTexture(texture_filename))){
		return false;
	}

	return true;
}

bool Font::LoadFontData(WCHAR* filename) {

	// Create the font spacing buffer.
	m_Font = new FontType[95];
	if (!m_Font) {
		return false;
	}

	std::ifstream fin;
	// Read in the font size and spacing between chars.
	fin.open(filename);
	if (fin.fail()) {
		return false;
	}

	auto temp = ' ';
	// Read in the 95 used ascii characters for text.
	for (auto i = 0; i < 95; i++) {
		fin.get(temp);
		while (temp != ' ') {
			fin.get(temp);
		}
		fin.get(temp);
		while (temp != ' ') {
			fin.get(temp);
		}

		fin >> m_Font[i].left_;
		fin >> m_Font[i].right_;
		fin >> m_Font[i].size_;
	}

	// Close the file.
	fin.close();

	return true;
}

bool Font::LoadTexture(WCHAR* filename) {

	// Create the texture object.
	m_texture = std::make_shared<Texture>();
	if (!m_texture) {
		return false;
	}

	// Initialize the texture object.
	if (CHECK(m_texture->set_texture(filename))) {
		return false;
	}

	return true;
}

void Font::BuildVertexArray(void* vertices, WCHAR* sentence, float drawX, float drawY) {

	// Coerce the input vertices into a VertexType structure.
	VertexType *vertexPtr = (VertexType*)vertices;

	// Get the number of letters in the sentence.
	auto numLetters = wcslen(sentence);

	// Initialize the index to the vertex array.
	auto index = 0;

	auto letter = 0;

	// Draw each letter onto a quad.
	for (auto i = 0; i < numLetters; ++i) {

		letter = ((int)sentence[i]) - 32;

		// If the letter is a space then just move over three pixels.
		if (letter == 0) {
			drawX = drawX + 3.0f;
		}
		else {
			// First triangle in quad.
			// Top left.
			vertexPtr[index].position_ = DirectX::XMFLOAT3(drawX, drawY, 0.0f);
			vertexPtr[index].texture_ = DirectX::XMFLOAT2(m_Font[letter].left_, 0.0f);
			++index;

			// Bottom right.
			vertexPtr[index].position_ = DirectX::XMFLOAT3((drawX + m_Font[letter].size_), (drawY - 16), 0.0f);
			vertexPtr[index].texture_ = DirectX::XMFLOAT2(m_Font[letter].right_, 1.0f);
			++index;

			// Bottom left.
			vertexPtr[index].position_ = DirectX::XMFLOAT3(drawX, (drawY - 16), 0.0f);
			vertexPtr[index].texture_ = DirectX::XMFLOAT2(m_Font[letter].left_, 1.0f);
			++index;

			// Second triangle in quad.
			// Top left.
			vertexPtr[index].position_ = DirectX::XMFLOAT3(drawX, drawY, 0.0f);
			vertexPtr[index].texture_ = DirectX::XMFLOAT2(m_Font[letter].left_, 0.0f);
			++index;

			// Top right.
			vertexPtr[index].position_ = DirectX::XMFLOAT3(drawX + m_Font[letter].size_, drawY, 0.0f);
			vertexPtr[index].texture_ = DirectX::XMFLOAT2(m_Font[letter].right_, 0.0f);
			++index;

			// Bottom right.
			vertexPtr[index].position_ = DirectX::XMFLOAT3((drawX + m_Font[letter].size_), (drawY - 16), 0.0f);
			vertexPtr[index].texture_ = DirectX::XMFLOAT2(m_Font[letter].right_, 1.0f);
			++index;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + m_Font[letter].size_ + 1.0f;
		}
	}
}
