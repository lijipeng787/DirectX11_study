

#include "fontclass.h"

FontClass::FontClass() {
  font_ = 0;
  texture_ = nullptr;
}

FontClass::FontClass(const FontClass &other) {}

FontClass::~FontClass() {}

bool FontClass::Initialize(char *fontFilename, WCHAR *textureFilename) {
  bool result;

  // Load in the text file containing the font data.
  result = LoadFontData(fontFilename);
  if (!result) {
    return false;
  }

  // Load the texture that has the font characters on it.
  result = LoadTexture(textureFilename);
  if (!result) {
    return false;
  }

  return true;
}

void FontClass::Shutdown() {
  // Release the font texture.
  ReleaseTexture();

  // Release the font data.
  ReleaseFontData();
}

bool FontClass::LoadFontData(char *filename) {
  ifstream fin;
  int i;
  char temp;

  // Create the font spacing buffer.
  font_ = new FontType[95];
  if (!font_) {
    return false;
  }

  // Read in the font size and spacing between chars.
  fin.open(filename);
  if (fin.fail()) {
    return false;
  }

  // Read in the 95 used ascii characters for text.
  for (i = 0; i < 95; i++) {
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
  // Release the font data array.
  if (font_) {
    delete[] font_;
    font_ = 0;
  }
}

bool FontClass::LoadTexture(WCHAR *filename) {
  bool result;

  texture_ = new TextureClass();
  if (!texture_) {
    return false;
  }

  result = texture_->Initialize(filename);
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

ID3D11ShaderResourceView *FontClass::GetTexture() {
  return texture_->GetTexture();
}

void FontClass::BuildVertexArray(void *vertices, char *sentence, float drawX,
                                 float drawY) {
  VertexType *vertexPtr;
  int numLetters, index, i, letter;

  // Coerce the input vertices into a VertexType structure.
  vertexPtr = (VertexType *)vertices;

  numLetters = (int)strlen(sentence);

  // Initialize the index to the vertex array.
  index = 0;

  // Draw each letter onto a quad.
  for (i = 0; i < numLetters; i++) {
    letter = ((int)sentence[i]) - 32;

    // If the letter is a space then just move over three pixels.
    if (letter == 0) {
      drawX = drawX + 3.0f;
    } else {
      // First triangle in quad.
      vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f); // Top left.
      vertexPtr[index].texture = XMFLOAT2(font_[letter].left, 0.0f);
      index++;

      vertexPtr[index].position = XMFLOAT3((drawX + font_[letter].size),
                                           (drawY - 16), 0.0f); // Bottom right.
      vertexPtr[index].texture = XMFLOAT2(font_[letter].right, 1.0f);
      index++;

      vertexPtr[index].position =
          XMFLOAT3(drawX, (drawY - 16), 0.0f); // Bottom left.
      vertexPtr[index].texture = XMFLOAT2(font_[letter].left, 1.0f);
      index++;

      // Second triangle in quad.
      vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f); // Top left.
      vertexPtr[index].texture = XMFLOAT2(font_[letter].left, 0.0f);
      index++;

      vertexPtr[index].position =
          XMFLOAT3(drawX + font_[letter].size, drawY, 0.0f); // Top right.
      vertexPtr[index].texture = XMFLOAT2(font_[letter].right, 0.0f);
      index++;

      vertexPtr[index].position = XMFLOAT3((drawX + font_[letter].size),
                                           (drawY - 16), 0.0f); // Bottom right.
      vertexPtr[index].texture = XMFLOAT2(font_[letter].right, 1.0f);
      index++;

      // Update the x location for drawing by the size of the letter and one
      // pixel.
      drawX = drawX + font_[letter].size + 1.0f;
    }
  }
}