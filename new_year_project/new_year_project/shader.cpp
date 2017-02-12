#include "shader.h"

Shader::Shader(const Shader & copy){
	if (this != &copy) {
		this->compiled_shader_ = copy.compiled_shader_;
		this->valid_ = copy.valid_;
	}
}

Shader & Shader::operator=(const Shader & copy){
	if (this != &copy) {
		this->compiled_shader_ = copy.compiled_shader_;
		this->valid_ = copy.valid_;
	}
	return *this;
}

Shader::~Shader(){
}

void Shader::set_valid(){
	valid_ = true;
}

bool Shader::isValid(){
	return valid_;
}

D3D10BlobPtr Shader::get_compiled_shader(){
	return compiled_shader_;
}

const D3D10BlobPtr Shader::get_compiled_shader() const{
	return compiled_shader_;
}

VertexShader::VertexShader(const VertexShader & copy){
	if (this != &copy) {
		this->vertex_shader_ = copy.vertex_shader_;
		this->compiled_shader_ = copy.compiled_shader_;
		this->valid_ = copy.valid_;
	}
}

VertexShader & VertexShader::operator=(const VertexShader & copy){
	if (this != &copy) {
		this->vertex_shader_ = copy.vertex_shader_;
	}
	return *this;
}

VertexShader::~VertexShader(){
}

VertexShaderPtr VertexShader::get_vertex_shader(){
	return vertex_shader_;
}

const VertexShaderPtr VertexShader::get_vertex_shader() const{
	return vertex_shader_;
}

const ShaderType VertexShader::GetType() const{
	return ShaderType::kVertexShader;
}

PixelShader::PixelShader(const PixelShader & copy){
	if (this != &copy) {
		this->pixel_shader_ = copy.pixel_shader_;
	}
}

PixelShader & PixelShader::operator=(const PixelShader & copy){
	if (this != &copy) {
		this->pixel_shader_ = copy.pixel_shader_;
	}
	return *this;
}

PixelShader::~PixelShader(){
}

const ShaderType PixelShader::GetType() const{
	return ShaderType::kPixelShader;
}

PixelShaderPtr PixelShader::get_pixel_shader(){
	return pixel_shader_;
}

const PixelShaderPtr PixelShader::get_pixel_shader() const{
	return pixel_shader_;
}
