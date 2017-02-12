#pragma once

#ifndef GRAPHICS_HEADER_TERRAIN_H

#define GRAPHICS_HEADER_TERRAIN_H

#include "common.h"
#include "rendeable.h"
#include "terrain_cell.h"

class Terrain :Renderable {
private:
	typedef struct _HeightmapType {
		float x, y, z;

		float nx, ny, nz;

		float r, g, b;
	} HeightmapType;

	typedef struct _ModelType {
		float x, y, z;

		float tu, tv;

		float nx, ny, nz;

		float tx, ty, tz;

		float bx, by, bz;

		float r, g, b;
	} ModelType;

	typedef struct _VectorType {
		float x, y, z;
	} VectorType;

	typedef struct _TemVertexType {
		float x, y, z;

		float tu, tv;

		float nx, ny, nz;
	} TemVertexType;

public:
	Terrain();

	Terrain(const Terrain& copy);

	~Terrain();

public:
	bool Initialize(char *setup_filename);

	void Shutdown();

	bool RenderCell(int cell_id);

	void RenderCellLines(int cell_id);

	int GetCellIndexCount(int cell_id);

	int GetCellLinesIndexCount(int line_id);

	int get_terrain_cell_count();

private:
	bool LoadSetupFile(char *setup_filename);

	bool LoadRawHeightmap();

	void ShutdownHeightmap();

	void SetTerrainCoordinates();

	bool CalculateNormals();

	bool LoadColormap();

	bool BuildTerrainModel();

	void ShutdownTerrainModel();

	void CalculateTerrainVectors();

	void CalculateTangentBinormal(
		TemVertexType vertex1, 
		TemVertexType vertex2, 
		TemVertexType vertex3,
		VectorType& tangent, 
		VectorType& binormal
		);

	bool LoadTerrainCells();

	void ShutdownTerrainCells();

private:
	int terrain_height_, terrain_width_;

	int terrain_vertex_count_;

	float height_scale_;

	char *terrain_filename_;

	char *color_filename_;

	HeightmapType *heightmap_;

	ModelType *terrain_model_;

	TerrainCell *terrain_cells_array_;

	int terrain_cell_count_;
};

#endif // !GRAPHICS_HEADER_TERRAIN_H
