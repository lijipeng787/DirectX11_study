#pragma once

#ifndef GRAPHICS_HEADER_INPUT_LAYOUT_H

#define GRAPHICS_HEADER_INPUT_LAYOUT_H

#include <d3d11.h>

class InputLayoutDesc {
public:
	InputLayoutDesc();

	InputLayoutDesc(const InputLayoutDesc& copy);

	~InputLayoutDesc();

public:
	void SetInputLayoutDesc(D3D11_INPUT_ELEMENT_DESC *input_layout_desc);

	void InitializeInputElementDesc(int num_element);
	
	const D3D11_INPUT_ELEMENT_DESC* get_input_layout_desc()const;

	const int get_num_element()const;

private:
	int num_element_ = 0;

	D3D11_INPUT_ELEMENT_DESC *input_layout_desc_ = nullptr;
};

#endif // !GRAPHICS_HEADER_INPUT_LAYOUT_H
