#include "buffer_config.h"

BufferConfig::BufferConfig(){
	ZeroMemory(&buffer_desc, sizeof(buffer_desc));
}

BufferConfig::BufferConfig(const BufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
}

BufferConfig & BufferConfig::operator=(const BufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
	return *this;
}

BufferConfig::~BufferConfig(){
}

void BufferConfig::SetByteWidth(UINT byte_width){
	buffer_desc.ByteWidth = byte_width;
}

void BufferConfig::SetBufferUsage(BufferUseage usage) {
	switch (usage) {
	case BufferUseage::kUsageDefault:
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		break;
	case BufferUseage::kUsageDynamic:
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		break;
	case BufferUseage::kUsageImmutable:
		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		break;
	case BufferUseage::kUsageStaging:
		buffer_desc.Usage = D3D11_USAGE_STAGING;
		break;
	default:
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		break;
	}
}

void BufferConfig::SetBindType(UINT bind_type){
	buffer_desc.BindFlags = bind_type;
}

void BufferConfig::SetCpuAccessType(UINT cpu_access_type){
	buffer_desc.CPUAccessFlags = cpu_access_type;
}

void BufferConfig::SetMiscFlags(UINT misc_flags){
	buffer_desc.MiscFlags = misc_flags;
}

void BufferConfig::SetStructreByteStride(UINT structre_byte_stride){
	buffer_desc.StructureByteStride = structre_byte_stride;
}

const D3D11_BUFFER_DESC* BufferConfig::get_buffer_desc()const{
	return &(this->buffer_desc);
}

VertexBufferConfig::VertexBufferConfig() {
	this->buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	this->buffer_desc.ByteWidth = 0;
	this->buffer_desc.CPUAccessFlags = 0;
	this->buffer_desc.MiscFlags = 0;
	this->buffer_desc.StructureByteStride = 0;
	this->buffer_desc.Usage = D3D11_USAGE_DEFAULT;
}

VertexBufferConfig::VertexBufferConfig(const VertexBufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
}

VertexBufferConfig & VertexBufferConfig::operator=(const VertexBufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
	return *this;
}

VertexBufferConfig::~VertexBufferConfig(){
}

void VertexBufferConfig::SetBindType(UINT bind_type){
	// warning:can not change the vertex buffer's bind flag
}

IndexBufferConfig::IndexBufferConfig(){
	this->buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	this->buffer_desc.ByteWidth = 0;
	this->buffer_desc.CPUAccessFlags = 0;
	this->buffer_desc.MiscFlags = 0;
	this->buffer_desc.StructureByteStride = 0;
	this->buffer_desc.Usage = D3D11_USAGE_DEFAULT;
}

IndexBufferConfig::IndexBufferConfig(const IndexBufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
}

IndexBufferConfig & IndexBufferConfig::operator=(const IndexBufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
	return *this;
}

IndexBufferConfig::~IndexBufferConfig(){
}

void IndexBufferConfig::SetBindType(UINT bind_type){
	// warning:can not change the index buffer's bind flag
}

ConstantBufferConfig::ConstantBufferConfig(){
	this->buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	this->buffer_desc.ByteWidth = 0;
	this->buffer_desc.CPUAccessFlags = 0;
	this->buffer_desc.MiscFlags = 0;
	this->buffer_desc.StructureByteStride = 0;
	this->buffer_desc.Usage = D3D11_USAGE_DEFAULT;
}

ConstantBufferConfig::ConstantBufferConfig(const ConstantBufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
}

ConstantBufferConfig & ConstantBufferConfig::operator=(const ConstantBufferConfig & copy){
	if (this != &copy) {
		this->buffer_desc.BindFlags = copy.buffer_desc.BindFlags;
		this->buffer_desc.ByteWidth = copy.buffer_desc.ByteWidth;
		this->buffer_desc.CPUAccessFlags = copy.buffer_desc.CPUAccessFlags;
		this->buffer_desc.MiscFlags = copy.buffer_desc.MiscFlags;
		this->buffer_desc.StructureByteStride = copy.buffer_desc.StructureByteStride;
		this->buffer_desc.Usage = copy.buffer_desc.Usage;
	}
	return *this;
}

ConstantBufferConfig::~ConstantBufferConfig(){
}

void ConstantBufferConfig::SetBindType(UINT bind_type){
	// warning:can not change the constant buffer's bind flag
}
