#include "input_layout.h"
#include <assert.h>

InputLayoutDesc::InputLayoutDesc(){
}

InputLayoutDesc::InputLayoutDesc(const InputLayoutDesc & copy){
}

InputLayoutDesc::~InputLayoutDesc(){
}

void InputLayoutDesc::SetInputLayoutDesc(D3D11_INPUT_ELEMENT_DESC * input_layout_desc){
	this->input_layout_desc_ = input_layout_desc;
}

void InputLayoutDesc::InitializeInputElementDesc(int num_element){
	this->num_element_ = num_element;
	this->input_layout_desc_ = new D3D11_INPUT_ELEMENT_DESC[num_element];
	ZeroMemory(input_layout_desc_, sizeof(input_layout_desc_)*num_element);
}

const D3D11_INPUT_ELEMENT_DESC * InputLayoutDesc::get_input_layout_desc() const{
	return this->input_layout_desc_;
}

const int InputLayoutDesc::get_num_element() const{
	return this->num_element_;
}
