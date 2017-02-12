#pragma once

#ifndef GRAPHICS_HEADER_TERRAIN_CELL_H

#define GRAPHICS_HEADER_TERRAIN_CELL_H

#include "common.h"

class TerrainCell {

	typedef struct _ModelType {
		float x, y, z;

		float tu, tv;

		float nx, ny, nz;

		float tx, ty, tz;

		float bx, by, bz;

		float r, g, b;
	} ModelType;

	typedef struct _VertexType {
		DirectX::XMFLOAT3 position;

		DirectX::XMFLOAT2 texture;

		DirectX::XMFLOAT3 normal;

		DirectX::XMFLOAT3 tangent;

		DirectX::XMFLOAT3 binormal;

		DirectX::XMFLOAT3 color;
	} VertexType;

	typedef struct _VectorType {
		float x, y, z;
	} VectorType;

	typedef struct _ColorVertexType {
		DirectX::XMFLOAT3 position;

		DirectX::XMFLOAT4 color;
	} ColorVertexType;

public:
	TerrainCell();

	TerrainCell(const TerrainCell& copy);

	~TerrainCell();

public:
	bool Initialize(
		void *terrain_model,
		int node_index_x, int node_index_y,
		int cell_height, int cell_width,
		int terrain_width
		);

	void Shutdown();

	void Render();

	void RenderCellLine();

	int GetVertexCount();
	
	int GetIndexCount();

	int GetLineBufferIndexCount();

	void GetCellDimensions(
		float& max_width, float& max_height, float& max_depth,
		float& min_width, float& min_height, float& min_depth
		);

private:

	bool InitializeBuffers(
		int node_index_x, int node_index_y,
		int cell_height, int cell_width,
		int terrain_width,
		ModelType *terrain_model
		);

	void ShutdownBuffers();

	void RenderBuffers();

	void CalculateCellDimensions();

	bool BuildLineBuffers();

	void ShutdownLineBuffers();

private:
	VectorType *vertex_list_;

	int vertex_count_, index_count_, line_index_count_;

	ID3D11Buffer *vertex_buffer_, *index_buffer_, *line_vertex_buffer_, *line_index_buffer_;

	float max_width_, max_height_, max_depth_;
	
	float min_width_, min_height_, min_depth_;

	float cell_center_pos_x_, cell_center_pos_y_, cell_center_pos_z_;
};

#endif // !GRAPHICS_HEADER_TERRAIN_CELL_H
