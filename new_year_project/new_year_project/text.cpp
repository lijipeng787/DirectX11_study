#include "text.h"

#define OBJECT_ARR_CREATE_AND_TEST(obj_name,class_name,size)	\
		auto obj_name=new class_name[size];						\
		if(!obj_name){return false;}							\

#define OBJECT_SEAF_DELTE(object_name)	\
		if(object_name){				\
			object_name->Release();		\
			object_name=nullptr;		\
		}								\

Text::Text() :vertex_buffer_(nullptr), shadow_vertex_buffer_(nullptr),
index_buffer_(nullptr), shadow_index_buffer_(nullptr),
screen_width_(0), scrren_height_(0), max_length_(0), vertex_count_(0), index_count_(0),
shadow_(false),
pixel_color_(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f,1.0f)) {
}

Text::Text(const Text & copy){
}

Text::~Text(){
}

bool Text::Initialize(int screen_width, int screen_height, int max_length, bool shadow,
	Font * font_object, char * text, int position_x, int position_y,
	float red, float green, float blue) {

	this->scrren_height_ = screen_height;
	this->screen_width_ = screen_width;

	this->max_length_ = max_length;

	this->shadow_ = shadow;

	if (!(InitializeSentence(font_object, text,
		position_x, position_y,
		red, green, blue))) {
		return false;
	}

	return true;
}

void Text::Shutdown(){
	OBJECT_SEAF_DELTE(vertex_buffer_)
	OBJECT_SEAF_DELTE(shadow_vertex_buffer_)
	OBJECT_SEAF_DELTE(index_buffer_)
	OBJECT_SEAF_DELTE(shadow_index_buffer_)
}

void Text::Render(ShaderManager * shader_manager_object,
	DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX orthogonality, 
	ID3D11ShaderResourceView * srv_font_texture){
	
	RenderSentence(shader_manager_object, world, view, orthogonality, srv_font_texture);
}

bool Text::UpdateSentence(Font * font_object, char * text, 
	int position_x, int position_y, float red, float green, float blue){

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	pixel_color_ = DirectX::XMFLOAT4(red, green, blue, 1.0f);

	auto num_letters = strlen(text);

	if (num_letters > max_length_)
		return false;

	OBJECT_ARR_CREATE_AND_TEST(vertices, VertexType, vertex_count_);
	ZeroMemory(vertices, sizeof(VertexType)*vertex_count_);

	auto draw_x = static_cast<float>(-1 * (screen_width_ / 2) + position_x);
	auto draw_y = static_cast<float>((scrren_height_ / 2) - position_y);

	font_object->BuildVertexArray(static_cast<void*>(vertices), text, draw_x, draw_y);

	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	ZeroMemory(&mapped_subresource, sizeof(mapped_subresource));

	if (FAILED(device_context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource))) {
		return false;
	}

	auto data = static_cast<void*>(mapped_subresource.pData);
	memcpy(data, vertices, sizeof(VertexType)*vertex_count_);

	device_context->Unmap(vertex_buffer_, 0);

	if (shadow_) {
		ZeroMemory(vertices, sizeof(vertices));
		draw_x = static_cast<float>(-1 * (screen_width_ / 2) + position_x + 2);
		draw_y = static_cast<float>((scrren_height_ / 2) - position_y - 2);
		font_object->BuildVertexArray(static_cast<void*>(vertices), text, draw_x, draw_y);

		if (FAILED(device_context->Map(shadow_vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource))) {
			return false;
		}

		data = static_cast<void*>(mapped_subresource.pData);
		memcpy(data, vertices, sizeof(vertex_count_) * 6);

		device_context->Unmap(shadow_vertex_buffer_, 0);
	}

	delete[] vertices;
	vertices = nullptr;

	return true;
}

bool Text::InitializeSentence(Font * font_object, char * text, 
	int position_x, int position_y, float red, float green, float blue){

	auto device = D3d11Device::get_d3d_object()->get_device();
	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	this->vertex_count_ = 6 * this->max_length_;
	this->index_count_ = 6 * this->max_length_;

	OBJECT_ARR_CREATE_AND_TEST(vertices, VertexType, vertex_count_);

	OBJECT_ARR_CREATE_AND_TEST(indices, unsigned long, index_count_);

	for (int i = 0; i < index_count_; ++i) {
		indices[i] = i;
	}

	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(buffer_desc));

	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA buffer_data;
	ZeroMemory(&buffer_data, sizeof(buffer_data));

	buffer_data.pSysMem = vertices;
	buffer_data.SysMemPitch = 0;
	buffer_data.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &vertex_buffer_))) {
		return false;
	}

	buffer_desc.ByteWidth = sizeof(unsigned long)*index_count_;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	buffer_data.pSysMem = indices;
	buffer_data.SysMemPitch = 0;
	buffer_data.SysMemSlicePitch = 0;

	if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &index_buffer_))) {
		return false;
	}

	if (shadow_) {
		if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &shadow_index_buffer_))) {
			return false;
		}

		buffer_desc.ByteWidth = sizeof(VertexType)*vertex_count_;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ZeroMemory(&buffer_data, sizeof(buffer_data));
		if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &shadow_vertex_buffer_))) {
			return false;
		}
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	if (!(UpdateSentence(font_object, text, position_x, position_y, red, green, blue))) {
		return false;
	}

	return true;
} 

void Text::RenderSentence(ShaderManager * shader_manager_object,
	DirectX::XMMATRIX world, DirectX::XMMATRIX view, DirectX::XMMATRIX orthogonality, 
	ID3D11ShaderResourceView * srv_font_texture){

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	if (shadow_) {
		auto shadow_color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

		device_context->IASetVertexBuffers(0, 1, &shadow_vertex_buffer_, &stride, &offset);

		device_context->IASetIndexBuffer(shadow_index_buffer_, DXGI_FORMAT_R32_UINT, 0);

		device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		shader_manager_object->ExecuteFontShader(index_count_,world, view, orthogonality, srv_font_texture, shadow_color);
	}

	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader_manager_object->ExecuteFontShader(index_count_, world, view, orthogonality, srv_font_texture, pixel_color_);

}
