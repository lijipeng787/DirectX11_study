#include "fontclass.h"
#include "textureclass.h"

#include <DirectXMath.h>
#include <fstream>

using namespace std;
using namespace DirectX;

struct FontType {
	float left, right;
	int size;
};

struct VertexType {
	XMFLOAT3 position;
	XMFLOAT2 texture;
};

bool FontClass::Initialize(char* fontFilename, WCHAR* textureFilename) {

	auto result = LoadFontData(fontFilename);
	if (!result) {
		return false;
	}

	result = LoadTexture(textureFilename);
	if (!result) {
		return false;
	}

	return true;
}

void FontClass::Shutdown() {

	ReleaseTexture();

	ReleaseFontData();
}

bool FontClass::LoadFontData(char* filename) {

	font_ = new FontType[95];
	if (!font_) {
		return false;
	}

	ifstream fin;
	fin.open(filename);
	if (fin.fail()) {
		return false;
	}

	char temp = 0;
	// Read in the 95 used ascii characters for text.
	for (int i = 0; i < 95; i++) {
		fin.get(temp);
		while (temp != ' ') {
			fin.get(temp);
		}
		fin.get(temp);
		while (temp != ' ') {
			fin.get(temp);
		}

		fin >> font_[i].left;
		fin >> font_[i].right;
		fin >> font_[i].size;
	}

	fin.close();

	return true;
}

void FontClass::ReleaseFontData() {
	if (font_) {
		delete[] font_;
		font_ = nullptr;
	}
}

bool FontClass::LoadTexture(WCHAR* filename) {

	texture_ = new TextureClass();
	if (!texture_) {
		return false;
	}

	auto result = texture_->Initialize(filename);
	if (!result) {
		return false;
	}

	return true;
}

void FontClass::ReleaseTexture() {

	if (texture_) {
		texture_->Shutdown();
		delete texture_;
		texture_ = nullptr;
	}
}

ID3D11ShaderResourceView* FontClass::GetTexture() {
	return texture_->GetTexture();
}

void FontClass::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY) {

	// Coerce the input vertices into a VertexType structure.
	VertexType *vertexPtr = static_cast<VertexType*>(vertices);

	auto numLetters = static_cast<int>(strlen(sentence));

	// Initialize the index to the vertex array.
	int index = 0;

	int letter = 0;
	// Draw each letter onto a quad.
	for (int i = 0; i < numLetters; i++) {
		letter = static_cast<int>(sentence[i]) - 32;

		// If the letter is a space then just move over three pixels.
		if (letter == 0) {
			drawX = drawX + 3.0f;
		}
		else {
			// First triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(font_[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + font_[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(font_[letter].right, 1.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - 16), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(font_[letter].left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(font_[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX + font_[letter].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(font_[letter].right, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + font_[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(font_[letter].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + font_[letter].size + 1.0f;
		}
	}
}