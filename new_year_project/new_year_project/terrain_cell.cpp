#include "terrain_cell.h"

TerrainCell::TerrainCell() :vertex_buffer_(nullptr), index_buffer_(nullptr),
line_vertex_buffer_(nullptr), line_index_buffer_(nullptr),
vertex_count_(0), index_count_(0), line_index_count_(0), 
max_width_(-1000000.0f), max_height_(-1000000.0f), max_depth_(-1000000.0f),
min_width_(1000000.0f), min_height_(1000000.0f), min_depth_(1000000.0f), 
cell_center_pos_x_(0.0f), cell_center_pos_y_(0.0f), cell_center_pos_z_(0.0f) {
}

TerrainCell::TerrainCell(const TerrainCell & copy){
}

TerrainCell::~TerrainCell(){
}

bool TerrainCell::Initialize(void * terrain_model, 
	int node_index_x, int node_index_y, 
	int cell_height, int cell_width, int terrain_width){
	if (!(InitializeBuffers(node_index_x, node_index_y,cell_height, cell_width,
		terrain_width, static_cast<ModelType*>(terrain_model)))) {
		return false;
	}

	CalculateCellDimensions();

	if (!(BuildLineBuffers())) {
		return false;
	}

	return true;
}

void TerrainCell::Shutdown(){
	ShutdownLineBuffers();

	ShutdownBuffers();
}

void TerrainCell::Render(){
	RenderBuffers();
}

void TerrainCell::RenderCellLine(){

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	UINT stride = sizeof(ColorVertexType);
	UINT offset = 0;

	device_context->IASetVertexBuffers(0, 1, &line_vertex_buffer_, &stride, &offset);

	device_context->IASetIndexBuffer(line_index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

int TerrainCell::GetVertexCount(){
	return vertex_count_;
}

int TerrainCell::GetIndexCount(){
	return index_count_;
}

int TerrainCell::GetLineBufferIndexCount(){
	return line_index_count_;
}

void TerrainCell::GetCellDimensions(float & max_width, float & max_height, float & max_depth,
	float & min_width, float & min_height, float & min_depth){

	max_height = this->max_height_;
	max_width = this->max_width_;
	max_depth = this->max_depth_;

	min_height = this->min_height_;
	min_width = this->min_width_;
	min_depth = this->min_depth_;
}

bool TerrainCell::InitializeBuffers(int node_index_x, int node_index_y, 
	int cell_height, int cell_width, int terrain_width, ModelType * terrain_model){

	using namespace DirectX;

	auto device = D3d11Device::get_d3d_object()->get_device();

	vertex_count_ = (cell_height - 1)*(cell_width - 1) * 6;

	index_count_ = vertex_count_;

	auto vertices = new VertexType[vertex_count_];
	if (!vertices)
		return false;

	auto indices = new unsigned long[index_count_];
	if (!indices)
		return false;

	auto model_index = ((node_index_x*(cell_width - 1)) + (node_index_y*(cell_height - 1)*(terrain_width - 1))) * 6;
	auto index = 0;
	for (int i = 0; i < (cell_height - 1); ++i) {
		for (int j = 0; j < (cell_width - 1) * 6; ++j) {
			vertices[index].position = XMFLOAT3(terrain_model[model_index].x, terrain_model[model_index].y, terrain_model[model_index].z);

			vertices[index].texture = XMFLOAT2(terrain_model[model_index].tu, terrain_model[model_index].tv);

			vertices[index].normal = XMFLOAT3(terrain_model[model_index].nx, terrain_model[model_index].ny, terrain_model[model_index].nz);

			vertices[index].tangent = XMFLOAT3(terrain_model[model_index].tx, terrain_model[model_index].ty, terrain_model[model_index].tz);

			vertices[index].binormal = XMFLOAT3(terrain_model[model_index].bx, terrain_model[model_index].by, terrain_model[model_index].bz);

			vertices[index].color = XMFLOAT3(terrain_model[model_index].r, terrain_model[model_index].g, terrain_model[model_index].b);

			indices[index] = index;
			++index;
			model_index++;
		}
		model_index += (terrain_width * 6) - (cell_width * 6);
	}

	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(buffer_desc));

	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
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

	vertex_list_ = new VectorType[vertex_count_];
	if (!vertex_list_)
		return false;

	for (int i = 0; i < vertex_count_; ++i) {
		vertex_list_[i].x = vertices[i].position.x;
		vertex_list_[i].y = vertices[i].position.y;
		vertex_list_[i].z = vertices[i].position.z;
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void TerrainCell::ShutdownBuffers(){
	if (vertex_list_){
		delete[] vertex_list_;
		vertex_list_ = nullptr;
	}

	if (index_buffer_){
		index_buffer_->Release();
		index_buffer_ = nullptr;
	}

	if (vertex_buffer_){
		vertex_buffer_->Release();
		vertex_buffer_ = nullptr;
	}

}

void TerrainCell::RenderBuffers(){
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	auto device_context = D3d11Device::get_d3d_object()->get_device_context();

	device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

	device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void TerrainCell::CalculateCellDimensions(){

	auto width = 0.0f;

	auto height = 0.0f;

	auto depth = 0.0f;

	for (int i = 0; i < vertex_count_; ++i) {
		width = vertex_list_[i].x;
		height = vertex_list_[i].y;
		depth = vertex_list_[i].z;

		max_width_ = (width > max_width_) ? width : max_width_;

		min_width_ = (width < min_width_) ? width : min_width_;

		max_height_ = (height > max_height_) ? height : max_height_;

		min_height_ = (height < min_height_) ? height : min_height_;

		max_depth_ = (depth > max_depth_) ? depth : max_depth_;

		min_depth_ = (depth < min_depth_) ? depth : min_depth_;
	}

	cell_center_pos_x_ = (max_width_ - min_width_) + min_width_;

	cell_center_pos_y_ = (max_height_ - min_height_) + min_height_;

	cell_center_pos_z_ = (max_depth_ - min_depth_) + min_depth_;
}

bool TerrainCell::BuildLineBuffers(){
	using namespace DirectX;

	auto device = D3d11Device::get_d3d_object()->get_device();

	auto line_color = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);

	auto cell_vertex_count = 24;

	auto cell_index_count = cell_vertex_count;

	auto cell_vertices = new ColorVertexType[cell_vertex_count];
	if (!cell_vertices)
		return false;

	auto cell_indeces = new unsigned long[cell_index_count];
	if (!cell_index_count)
		return false;

	D3D11_BUFFER_DESC buffer_desc;
	ZeroMemory(&buffer_desc, sizeof(buffer_desc));

	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(ColorVertexType) * cell_vertex_count;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA buffer_data;
	ZeroMemory(&buffer_data, sizeof(buffer_data));

	buffer_data.pSysMem = cell_vertices;
	buffer_data.SysMemPitch = 0;
	buffer_data.SysMemSlicePitch = 0;

	auto index = 0;

	cell_vertices[index].position = XMFLOAT3(min_width_, min_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, min_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, min_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, min_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, min_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, min_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, min_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, min_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, max_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, max_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, max_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, max_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, max_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, max_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, max_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, max_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	// 4 Verticle lines.
	cell_vertices[index].position = XMFLOAT3(max_width_, max_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, min_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, max_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, min_height_, max_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, max_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(max_width_, min_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, max_height_, min_depth_);
	cell_vertices[index].color = line_color;
	index++;

	cell_vertices[index].position = XMFLOAT3(min_width_, min_height_, min_depth_);
	cell_vertices[index].color = line_color;

	if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &line_vertex_buffer_))) {
		return false;
	}

	buffer_desc.ByteWidth = sizeof(unsigned long)*cell_index_count;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	buffer_data.pSysMem = cell_indeces;

	for (int i = 0; i < cell_index_count; ++i) {
		cell_indeces[i] = i;
	}

	if (FAILED(device->CreateBuffer(&buffer_desc, &buffer_data, &line_index_buffer_))) {
		return false;
	}

	line_index_count_ = cell_index_count;

	delete[] cell_vertices;
	cell_vertices = nullptr;

	delete[] cell_indeces;
	cell_indeces = nullptr;

	return true;
}

void TerrainCell::ShutdownLineBuffers(){
	if (line_index_buffer_) {
		line_index_buffer_->Release();
		line_index_buffer_ = nullptr;
	}

	if (line_vertex_buffer_) {
		line_vertex_buffer_->Release();
		line_vertex_buffer_ = nullptr;
	}
}
