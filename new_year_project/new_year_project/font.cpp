#include "font.h"

Font::Font() :font_property_(nullptr), texture_value_(nullptr), font_height_(0.0f), space_size_(0) {
}

Font::Font(const Font & copy){
}

Font::~Font(){
}

bool Font::Initialize(char * font_filename, char * texture_filename, float font_height, int space_size){
	this->font_height_ = font_height;

	this->space_size_ = space_size;

	if (!(LoadFontData(font_filename))) {
		return false;
	}

	if (!(LoadTexture(texture_filename))) {
		return false;
	}

	return true;
}

void Font::Shutdown(){
	ReleaseTexture();

	ReleaseFontData();
}

ID3D11ShaderResourceView * Font::GetTexture(){
	return texture_value_->GetTexture();
}

void Font::BuildVertexArray(void * vertices, char * sentence, float draw_pos_x, float draw_pos_y) {
	using namespace DirectX;

	auto vertex_ptr = static_cast<FontVertexType*> (vertices);

	auto num_letters = strlen(sentence);

	auto index = 0;

	auto letter = 0;

	for (int i = 0; i < num_letters; ++i) {
		letter = ((int)sentence[i]) - 32;

		if (letter == 0) {
			draw_pos_x = draw_pos_x + (float)space_size_;
		}
		else {
			// First triangle in quad.
			// Top left.
			vertex_ptr[index].position = XMFLOAT3(draw_pos_x, draw_pos_y, 0.0f);
			vertex_ptr[index].texture = XMFLOAT2(font_property_[letter].left, 0.0f);
			++index;

			// Bottom right.
			vertex_ptr[index].position = XMFLOAT3((draw_pos_x + font_property_[letter].size), (draw_pos_y - font_height_), 0.0f);
			vertex_ptr[index].texture = XMFLOAT2(font_property_[letter].right, 1.0f);
			++index;

			// Bottom left.
			vertex_ptr[index].position = XMFLOAT3(draw_pos_x, (draw_pos_y - font_height_), 0.0f);
			vertex_ptr[index].texture = XMFLOAT2(font_property_[letter].left, 1.0f);
			++index;

			// Second triangle in quad.
			// Top left.
			vertex_ptr[index].position = XMFLOAT3(draw_pos_x, draw_pos_y, 0.0f);
			vertex_ptr[index].texture = XMFLOAT2(font_property_[letter].left, 0.0f);
			++index;

			// Top right.
			vertex_ptr[index].position = XMFLOAT3(draw_pos_x + font_property_[letter].size, draw_pos_y, 0.0f);
			vertex_ptr[index].texture = XMFLOAT2(font_property_[letter].right, 0.0f);
			++index;

			// Bottom right.
			vertex_ptr[index].position = XMFLOAT3((draw_pos_x + font_property_[letter].size), (draw_pos_y - font_height_), 0.0f);
			vertex_ptr[index].texture = XMFLOAT2(font_property_[letter].right, 1.0f);
			++index;

			draw_pos_x = draw_pos_x + font_property_[letter].size + 1.0f;
		}
	}
}

int Font::GetSentencePixelLength(char * sentence){

	auto pixel_length = 0;

	auto num_letters = strlen(sentence);

	for (int i = 0; i < num_letters; ++i) {
		auto letter = sentence[i] - 32;
		if (0 == letter) {
			pixel_length += space_size_;
		}
		else {
			pixel_length += font_property_[letter].size + 1;
		}
	}
	return pixel_length;
}

int Font::get_font_height(){
	return static_cast<int>(font_height_);
}

bool Font::LoadFontData(char * file_name){

	font_property_ = new FontType[95];
	if (!font_property_) {
		return false;
	}

	std::ifstream fin;
	fin.open(file_name);
	if (fin.fail())
		return false;

	char read_bit = 0;
	for (int i = 0; i < 95; ++i) {
		fin.get(read_bit);
		while (' ' != read_bit) {
			fin.get(read_bit);
		}
		fin.get(read_bit);
		while (' ' != read_bit) {
			fin.get(read_bit);
		}

		fin >> font_property_[i].left;
		fin >> font_property_[i].right;
		fin >> font_property_[i].size;
	}

	fin.close();

	return true;
}

void Font::ReleaseFontData(){
	if (font_property_) {
		delete[] font_property_;
		font_property_ = nullptr;
	}
}

bool Font::LoadTexture(char * texture_filename){
	texture_value_ = new Texture;
	if (!texture_value_)
		return false;

	if (!(texture_value_->Initialize(texture_filename))) {
		return false;
	}

	return true;
}

void Font::ReleaseTexture(){
	if (texture_value_) {
		texture_value_->Shutdown();
		delete texture_value_;
		texture_value_ = nullptr;
	}
}
