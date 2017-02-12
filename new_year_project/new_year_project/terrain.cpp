#include "terrain.h"

Terrain::Terrain() :terrain_height_(0), terrain_width_(0), 
terrain_vertex_count_(0),height_scale_(0.0f), 
terrain_filename_(nullptr), color_filename_(nullptr), terrain_model_(nullptr),
terrain_cells_array_(nullptr), terrain_cell_count_(0),heightmap_(nullptr) {
}

Terrain::Terrain(const Terrain & copy){
}

Terrain::~Terrain(){
}

bool Terrain::Initialize(char * setup_filename){

	if (!(LoadSetupFile(setup_filename))) {
		return false;
	}

	if (!(LoadRawHeightmap())) {
		return false;
	}

	SetTerrainCoordinates();

	if (!(CalculateNormals())) {
		return false;
	}

	if (!(LoadColormap())) {
		return false;
	}

	if (!(BuildTerrainModel())) {
		return false;
	}

	ShutdownHeightmap();

	CalculateTerrainVectors();

	if (!(LoadTerrainCells())) {
		return false;
	}

	ShutdownTerrainModel();

	return true;

}

void Terrain::Shutdown(){
	ShutdownTerrainCells();

	ShutdownTerrainModel();

	ShutdownHeightmap();
}

bool Terrain::RenderCell(int cell_id){
	terrain_cells_array_[cell_id].Render();
	return true;
}

void Terrain::RenderCellLines(int cell_id){
	terrain_cells_array_[cell_id].RenderCellLine();
}

int Terrain::GetCellIndexCount(int cell_id){
	return terrain_cells_array_[cell_id].GetIndexCount();
}

int Terrain::GetCellLinesIndexCount(int cell_id){
	return terrain_cells_array_[cell_id].GetLineBufferIndexCount();
}

int Terrain::get_terrain_cell_count(){
	return terrain_cell_count_;
}

bool Terrain::LoadSetupFile(char * setup_filename){

	auto string_length = 256;

	terrain_filename_ = new char[string_length];
	if (!terrain_filename_)
		return false;
	color_filename_ = new char[string_length];
	if (!color_filename_)
		return false;

	std::ifstream fin;
	fin.open(setup_filename);
	if (fin.fail())
		return false;

	auto read_bit = ' ';
	fin.get(read_bit);
	while (':' != read_bit) {
		fin.get(read_bit);
	}
	fin >> terrain_filename_;

	fin.get(read_bit);
	while (':' != read_bit) {
		fin.get(read_bit);
	}
	fin >> terrain_height_;

	fin.get(read_bit);
	while (':'!= read_bit) {
		fin.get(read_bit);
	}
	fin >> terrain_width_;

	fin.get(read_bit);
	while (':' != read_bit) {
		fin.get(read_bit);
	}
	fin >> height_scale_;

	fin.get(read_bit);
	while (':' != read_bit) {
		fin.get(read_bit);
	}
	fin >> color_filename_;

	fin.clear();

	return true;
}

bool Terrain::LoadRawHeightmap(){

	heightmap_ = new HeightmapType[terrain_width_ * terrain_height_];
	if (!heightmap_) {
		return false;
	}

	FILE* filePtr;
	auto error = fopen_s(&filePtr, terrain_filename_, "rb");
	if (error != 0) {
		return false;
	}

	auto image_size = terrain_height_ * terrain_width_;

	auto raw_image = new unsigned short[image_size];
	if (!raw_image) {
		return false;
	}

	auto count = fread(raw_image, sizeof(unsigned short), image_size, filePtr);
	if (count != image_size) {
		return false;
	}

	error = fclose(filePtr);
	if (error != 0) {
		return false;
	}

	auto index = 0;
	for (auto j = 0; j<terrain_height_; j++) {
		for (auto i = 0; i<terrain_width_; i++) {
			index = (terrain_width_ * j) + i;
			heightmap_[index].y = (float)raw_image[index];
		}
	}

	delete[] raw_image;
	raw_image = 0;

	delete[] terrain_filename_;
	terrain_filename_ = nullptr;
	
	return true;
}

void Terrain::ShutdownHeightmap(){
	if (heightmap_) {
		delete[] heightmap_;
		heightmap_ = nullptr;
	}
}

void Terrain::SetTerrainCoordinates(){

	auto index = 0;
	for (auto j = 0; j<terrain_height_; j++){
		for (auto i = 0; i<terrain_width_; i++){
			index = (terrain_width_ * j) + i;

			heightmap_[index].x = (float)i;

			heightmap_[index].z = -(float)j;

			heightmap_[index].z += (float)(terrain_height_ - 1);

			heightmap_[index].y /= height_scale_;
		}
	}
}

// todo 1 rewrite the function to calculate normals each face in terain moedl
bool Terrain::CalculateNormals() {

	auto index1 = 0, index2 = 0, index3 = 0, index = 0;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;


	auto normals = new VectorType[(terrain_height_ - 1) * (terrain_width_ - 1)];
	if (!normals) {
		return false;
	}

	for (auto j = 0; j < (terrain_height_ - 1); j++) {
		for (auto i = 0; i < (terrain_width_ - 1); i++) {
			// Bottom left vertex.
			index1 = ((j + 1) * terrain_width_) + i;
			// Bottom right vertex.
			index2 = ((j + 1) * terrain_width_) + (i + 1);
			// Upper left vertex.
			index3 = (j * terrain_width_) + i;

			// Get three vertices from the face.
			vertex1[0] = heightmap_[index1].x;
			vertex1[1] = heightmap_[index1].y;
			vertex1[2] = heightmap_[index1].z;

			vertex2[0] = heightmap_[index2].x;
			vertex2[1] = heightmap_[index2].y;
			vertex2[2] = heightmap_[index2].z;

			vertex3[0] = heightmap_[index3].x;
			vertex3[1] = heightmap_[index3].y;
			vertex3[2] = heightmap_[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (terrain_width_ - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);

			// Calculate the length.
			length = (float)sqrt((normals[index].x * normals[index].x) + (normals[index].y * normals[index].y) +
				(normals[index].z * normals[index].z));

			// Normalize the final value for this face using the length.
			normals[index].x = (normals[index].x / length);
			normals[index].y = (normals[index].y / length);
			normals[index].z = (normals[index].z / length);
		}
	}

	// Now go through all the vertices and take a sum of the face normals that touch this vertex.
	for (auto j = 0; j < terrain_height_; j++) {
		for (auto i = 0; i < terrain_width_; i++) {
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0)) {
				index = ((j - 1) * (terrain_width_ - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Bottom right face.
			if ((i < (terrain_width_ - 1)) && ((j - 1) >= 0)) {
				index = ((j - 1) * (terrain_width_ - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (terrain_height_ - 1))) {
				index = (j * (terrain_width_ - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Upper right face.
			if ((i < (terrain_width_ - 1)) && (j < (terrain_height_ - 1))) {
				index = (j * (terrain_width_ - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
			}

			// Calculate the length of this normal.
			length = (float)sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * terrain_width_) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			heightmap_[index].nx = (sum[0] / length);
			heightmap_[index].ny = (sum[1] / length);
			heightmap_[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;

	return true;
}

bool Terrain::LoadColormap() {

	FILE* filePtr;
	auto error = fopen_s(&filePtr, color_filename_, "rb");
	if (error != 0) {
		return false;
	}

	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	auto count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if (count != 1) {
		return false;
	}

	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if (count != 1) {
		return false;
	}

	if ((bitmapInfoHeader.biWidth != terrain_width_) || (bitmapInfoHeader.biHeight != terrain_height_)) {
		return false;
	}

	auto image_size = terrain_height_ * ((terrain_width_ * 3) + 1);

	auto bitmap_image = new unsigned char[image_size];
	if (!bitmap_image) {
		return false;
	}

	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	count = fread(bitmap_image, 1, image_size, filePtr);
	if (count != image_size) {
		return false;
	}

	error = fclose(filePtr);
	if (error != 0) {
		return false;
	}

	auto k = 0;
	auto index = 0;

	for (auto j = 0; j < terrain_height_; j++) {
		for (auto i = 0; i < terrain_width_; i++) {
			index = (terrain_width_ * (terrain_height_ - 1 - j)) + i;

			heightmap_[index].b = (float)bitmap_image[k] / 255.0f;
			heightmap_[index].g = (float)bitmap_image[k + 1] / 255.0f;
			heightmap_[index].r = (float)bitmap_image[k + 2] / 255.0f;

			k += 3;
		}

		k++;
	}

	delete[] bitmap_image;
	bitmap_image = 0;

	return true;
}

bool Terrain::BuildTerrainModel() {

	terrain_vertex_count_ = (terrain_height_ - 1) * (terrain_width_ - 1) * 6;

	terrain_model_ = new ModelType[terrain_vertex_count_];
	if (!terrain_model_) {
		return false;
	}

	auto index = 0;
	auto index1 = 0, index2 = 0, index3 = 0, index4 = 0;
	for (auto j = 0; j < (terrain_height_ - 1); j++){
		for (auto i = 0; i < (terrain_width_ - 1); i++){
			// Get the indexes to the four points of the quad.
			// Upper left.
			index1 = (terrain_width_ * j) + i;
			// Upper right.
			index2 = (terrain_width_ * j) + (i + 1);
			// Bottom left.
			index3 = (terrain_width_ * (j + 1)) + i;
			// Bottom right.
			index4 = (terrain_width_ * (j + 1)) + (i + 1);

			// Now create two triangles for that quad.
			// Triangle 1 - Upper left.
			terrain_model_[index].x = heightmap_[index1].x;
			terrain_model_[index].y = heightmap_[index1].y;
			terrain_model_[index].z = heightmap_[index1].z;
			terrain_model_[index].tu = 0.0f;
			terrain_model_[index].tv = 0.0f;
			terrain_model_[index].nx = heightmap_[index1].nx;
			terrain_model_[index].ny = heightmap_[index1].ny;
			terrain_model_[index].nz = heightmap_[index1].nz;
			terrain_model_[index].r = heightmap_[index1].r;
			terrain_model_[index].g = heightmap_[index1].g;
			terrain_model_[index].b = heightmap_[index1].b;
			index++;

			// Triangle 1 - Upper right.
			terrain_model_[index].x = heightmap_[index2].x;
			terrain_model_[index].y = heightmap_[index2].y;
			terrain_model_[index].z = heightmap_[index2].z;
			terrain_model_[index].tu = 1.0f;
			terrain_model_[index].tv = 0.0f;
			terrain_model_[index].nx = heightmap_[index2].nx;
			terrain_model_[index].ny = heightmap_[index2].ny;
			terrain_model_[index].nz = heightmap_[index2].nz;
			terrain_model_[index].r = heightmap_[index2].r;
			terrain_model_[index].g = heightmap_[index2].g;
			terrain_model_[index].b = heightmap_[index2].b;
			index++;

			// Triangle 1 - Bottom left.
			terrain_model_[index].x = heightmap_[index3].x;
			terrain_model_[index].y = heightmap_[index3].y;
			terrain_model_[index].z = heightmap_[index3].z;
			terrain_model_[index].tu = 0.0f;
			terrain_model_[index].tv = 1.0f;
			terrain_model_[index].nx = heightmap_[index3].nx;
			terrain_model_[index].ny = heightmap_[index3].ny;
			terrain_model_[index].nz = heightmap_[index3].nz;
			terrain_model_[index].r = heightmap_[index3].r;
			terrain_model_[index].g = heightmap_[index3].g;
			terrain_model_[index].b = heightmap_[index3].b;
			index++;

			// Triangle 2 - Bottom left.
			terrain_model_[index].x = heightmap_[index3].x;
			terrain_model_[index].y = heightmap_[index3].y;
			terrain_model_[index].z = heightmap_[index3].z;
			terrain_model_[index].tu = 0.0f;
			terrain_model_[index].tv = 1.0f;
			terrain_model_[index].nx = heightmap_[index3].nx;
			terrain_model_[index].ny = heightmap_[index3].ny;
			terrain_model_[index].nz = heightmap_[index3].nz;
			terrain_model_[index].r = heightmap_[index3].r;
			terrain_model_[index].g = heightmap_[index3].g;
			terrain_model_[index].b = heightmap_[index3].b;
			index++;

			// Triangle 2 - Upper right.
			terrain_model_[index].x = heightmap_[index2].x;
			terrain_model_[index].y = heightmap_[index2].y;
			terrain_model_[index].z = heightmap_[index2].z;
			terrain_model_[index].tu = 1.0f;
			terrain_model_[index].tv = 0.0f;
			terrain_model_[index].nx = heightmap_[index2].nx;
			terrain_model_[index].ny = heightmap_[index2].ny;
			terrain_model_[index].nz = heightmap_[index2].nz;
			terrain_model_[index].r = heightmap_[index2].r;
			terrain_model_[index].g = heightmap_[index2].g;
			terrain_model_[index].b = heightmap_[index2].b;
			index++;

			// Triangle 2 - Bottom right.
			terrain_model_[index].x = heightmap_[index4].x;
			terrain_model_[index].y = heightmap_[index4].y;
			terrain_model_[index].z = heightmap_[index4].z;
			terrain_model_[index].tu = 1.0f;
			terrain_model_[index].tv = 1.0f;
			terrain_model_[index].nx = heightmap_[index4].nx;
			terrain_model_[index].ny = heightmap_[index4].ny;
			terrain_model_[index].nz = heightmap_[index4].nz;
			terrain_model_[index].r = heightmap_[index4].r;
			terrain_model_[index].g = heightmap_[index4].g;
			terrain_model_[index].b = heightmap_[index4].b;
			index++;
		}
	}

	return true;
}

void Terrain::ShutdownTerrainModel(){
	if (terrain_model_) {
		delete[] terrain_model_;
		terrain_model_ = nullptr;
	}
}

void Terrain::CalculateTerrainVectors() {

	auto face_count = terrain_vertex_count_ / 3;
	auto index = 0;
	TemVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal;

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for (auto i = 0; i < face_count; i++) {
		// Get the three vertices for this face from the terrain model.
		vertex1.x = terrain_model_[index].x;
		vertex1.y = terrain_model_[index].y;
		vertex1.z = terrain_model_[index].z;
		vertex1.tu = terrain_model_[index].tu;
		vertex1.tv = terrain_model_[index].tv;
		vertex1.nx = terrain_model_[index].nx;
		vertex1.ny = terrain_model_[index].ny;
		vertex1.nz = terrain_model_[index].nz;
		index++;

		vertex2.x = terrain_model_[index].x;
		vertex2.y = terrain_model_[index].y;
		vertex2.z = terrain_model_[index].z;
		vertex2.tu = terrain_model_[index].tu;
		vertex2.tv = terrain_model_[index].tv;
		vertex2.nx = terrain_model_[index].nx;
		vertex2.ny = terrain_model_[index].ny;
		vertex2.nz = terrain_model_[index].nz;
		index++;

		vertex3.x = terrain_model_[index].x;
		vertex3.y = terrain_model_[index].y;
		vertex3.z = terrain_model_[index].z;
		vertex3.tu = terrain_model_[index].tu;
		vertex3.tv = terrain_model_[index].tv;
		vertex3.nx = terrain_model_[index].nx;
		vertex3.ny = terrain_model_[index].ny;
		vertex3.nz = terrain_model_[index].nz;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

		// Store the tangent and binormal for this face back in the model structure.
		terrain_model_[index - 1].tx = tangent.x;
		terrain_model_[index - 1].ty = tangent.y;
		terrain_model_[index - 1].tz = tangent.z;
		terrain_model_[index - 1].bx = binormal.x;
		terrain_model_[index - 1].by = binormal.y;
		terrain_model_[index - 1].bz = binormal.z;

		terrain_model_[index - 2].tx = tangent.x;
		terrain_model_[index - 2].ty = tangent.y;
		terrain_model_[index - 2].tz = tangent.z;
		terrain_model_[index - 2].bx = binormal.x;
		terrain_model_[index - 2].by = binormal.y;
		terrain_model_[index - 2].bz = binormal.z;

		terrain_model_[index - 3].tx = tangent.x;
		terrain_model_[index - 3].ty = tangent.y;
		terrain_model_[index - 3].tz = tangent.z;
		terrain_model_[index - 3].bx = binormal.x;
		terrain_model_[index - 3].by = binormal.y;
		terrain_model_[index - 3].bz = binormal.z;
	}
}

void Terrain::CalculateTangentBinormal(TemVertexType vertex1, TemVertexType vertex2, TemVertexType vertex3, 
	VectorType & tangent, VectorType & binormal){
	
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];

	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	auto den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of the tangent.
	auto length = (float)sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the tangent and then store it.
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of the binormal.
	length = (float)sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the binormal and then store it.
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;
}

bool Terrain::LoadTerrainCells(){
	auto cell_width = 33;
	auto cell_height = 33;

	auto cell_row_count = (terrain_width_ - 1) / (cell_width - 1);
	terrain_cell_count_ = cell_row_count*cell_row_count;

	terrain_cells_array_ = new TerrainCell[terrain_cell_count_];
	if (!terrain_cells_array_)
		return false;

	auto index = 0;
	for (int i = 0; i < cell_row_count; ++i) {
		for (int j = 0; j < cell_row_count; ++j) {
			index = cell_row_count*i + j;

			if (!(terrain_cells_array_[index].Initialize(terrain_model_, j, i,
				cell_height, cell_width,terrain_width_))) {
				return false;
			}
		}
	}

	return true;
}

void Terrain::ShutdownTerrainCells(){
	if (terrain_cells_array_) {
		for (int i = 0; i < terrain_cell_count_; ++i)
			terrain_cells_array_[i].Shutdown();
	}
	delete[] terrain_cells_array_;
	terrain_cells_array_ = nullptr;
}
