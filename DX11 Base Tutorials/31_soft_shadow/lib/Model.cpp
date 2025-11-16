#include "Model.h"

#include "../../CommonFramework2/DirectX11Device.h"
#include "BoundingVolume.h"
#include "Interfaces.h"
#include "ShaderParameter.h"

#include <DirectXMath.h>
#include <algorithm>
#include <fstream>

using namespace std;
using namespace DirectX;

Model::~Model() { Shutdown(); }

bool Model::Initialize(const std::string &modelFilename,
                       const std::wstring &textureFilename,
                       ID3D11Device *device) {
  if (!LoadModel(modelFilename) || !InitializeBuffers(device) ||
      !LoadTexture(textureFilename, device)) {
    return false;
  }
  return true;
}

void Model::Shutdown() {
  ShutdownBuffers();
  ReleaseModel();
}

void Model::Render(const IShader &shader,
                   const ShaderParameterContainer &parameterContainer,
                   ID3D11DeviceContext *deviceContext) const {

  RenderBuffers(deviceContext);

  shader.Render(GetIndexCount(), parameterContainer, deviceContext);
}

void Model::SetParameterCallback(ShaderParameterCallback callback) {
  // Model doesn't use parameter callbacks by default
  // Derived classes can override if needed
}

ShaderParameterCallback Model::GetParameterCallback() const {
  // Return empty callback - Model uses default parameter behavior
  // This is safe and allows objects to render without custom parameter logic
  return nullptr;
}

ID3D11ShaderResourceView *Model::GetTexture() const {
  return texture_ ? texture_->GetTexture() : nullptr;
}

bool Model::InitializeBuffers(ID3D11Device *device) {
  // Check for overflow in buffer size calculation
  // Use (max)() to avoid Windows.h max macro conflict
  constexpr UINT MAX_BUFFER_SIZE = (std::numeric_limits<UINT>::max)();
  if (vertex_count_ > MAX_BUFFER_SIZE / sizeof(Vertex)) {
    std::cerr << "Error: Vertex count too large, would overflow buffer size calculation" << std::endl;
    return false;
  }
  if (index_count_ > MAX_BUFFER_SIZE / sizeof(unsigned long)) {
    std::cerr << "Error: Index count too large, would overflow buffer size calculation" << std::endl;
    return false;
  }

  std::vector<Vertex> vertices(vertex_count_);
  std::vector<unsigned long> indices(index_count_);

  for (int i = 0; i < vertex_count_; i++) {
    vertices[i].position = XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
    vertices[i].texture = XMFLOAT2(model_[i].tu, model_[i].tv);
    vertices[i].normal = XMFLOAT3(model_[i].nx, model_[i].ny, model_[i].nz);
    indices[i] = i;
  }

  D3D11_BUFFER_DESC vertexBufferDesc = {};
  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertex_count_);
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vertexData = {};
  vertexData.pSysMem = vertices.data();

  HRESULT result =
      device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertex_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC indexBufferDesc = {};
  indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned long) * index_count_);
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

  D3D11_SUBRESOURCE_DATA indexData = {};
  indexData.pSysMem = indices.data();

  result = device->CreateBuffer(&indexBufferDesc, &indexData, &index_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void Model::ShutdownBuffers() {
  index_buffer_.Reset();
  vertex_buffer_.Reset();
}

void Model::RenderBuffers(ID3D11DeviceContext *deviceContext) const {

  UINT stride = sizeof(Vertex);
  UINT offset = 0;

  deviceContext->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(),
                                    &stride, &offset);
  deviceContext->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::LoadTexture(const std::wstring &filename, ID3D11Device *device) {
  texture_ = std::make_unique<DDSTexture>();
  return texture_->Initialize(filename.c_str(), device);
}

bool Model::LoadModel(const std::string &filename) {
  std::ifstream fin(filename);
  if (!fin) {
    return false;
  }

  char input;
  // Add safety limit to prevent infinite loops on malformed files
  constexpr int MAX_PARSE_ITERATIONS = 10000;
  int iterations = 0;

  // Search for first delimiter with bounds checking
  while (fin.get(input) && input != ':' && ++iterations < MAX_PARSE_ITERATIONS) {
    // Continue searching
  }
  
  if (iterations >= MAX_PARSE_ITERATIONS || !fin) {
    std::cerr << "Error: Model file format error - first delimiter not found or file corrupted: " 
              << filename << std::endl;
    return false;
  }

  fin >> vertex_count_;
  
  // Validate vertex count to prevent excessive memory allocation or overflow
  constexpr int MAX_VERTEX_COUNT = 1000000; // Reasonable maximum
  if (vertex_count_ <= 0 || vertex_count_ > MAX_VERTEX_COUNT) {
    std::cerr << "Error: Invalid vertex count (" << vertex_count_ 
              << ") in model file: " << filename << std::endl;
    return false;
  }
  
  index_count_ = vertex_count_;
  model_.resize(vertex_count_);

  // Search for second delimiter with bounds checking
  iterations = 0;
  while (fin.get(input) && input != ':' && ++iterations < MAX_PARSE_ITERATIONS) {
    // Continue searching
  }
  
  if (iterations >= MAX_PARSE_ITERATIONS || !fin) {
    std::cerr << "Error: Model file format error - second delimiter not found: " 
              << filename << std::endl;
    return false;
  }
  
  fin.get(input);
  fin.get(input);

  // Read vertex data with stream state checking
  for (int i = 0; i < vertex_count_; i++) {
    fin >> model_[i].x >> model_[i].y >> model_[i].z;
    fin >> model_[i].tu >> model_[i].tv;
    fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;
    
    // Check for read errors after each vertex
    if (fin.fail()) {
      std::cerr << "Error: Failed to read vertex data at index " << i 
                << " in model file: " << filename << std::endl;
      return false;
    }
  }

  fin.close();

  // Calculate bounding volume
  CalculateBoundingVolume();

  return true;
}

void Model::CalculateBoundingVolume() {
  if (model_.empty()) {
    return;
  }

  std::vector<XMFLOAT3> positions;
  positions.reserve(model_.size());
  for (const auto &vertex : model_) {
    positions.push_back(XMFLOAT3(vertex.x, vertex.y, vertex.z));
  }

  bounding_volume_.CalculateFromVertices(positions.data(), positions.size());
}

const BoundingVolume &Model::GetLocalBoundingVolume() const {
  return bounding_volume_;
}

BoundingVolume Model::GetWorldBoundingVolume() const {
  return bounding_volume_.Transform(world_matrix_);
}

void Model::ReleaseModel() { model_.clear(); }

bool PBRModel::Initialize(const char *modelFilename,
                          const string &textureFilename1,
                          const string &textureFilename2,
                          const string &textureFilename3,
                          ID3D11Device *device) {

  auto result = LoadModel(modelFilename);
  if (!result) {
    return false;
  }

  // Calculate the tangent and binormal vectors for the model.
  CalculateModelVectors();

  result = InitializeBuffers(device);
  if (!result) {
    return false;
  }

  result = LoadTextures(textureFilename1, textureFilename2, textureFilename3,
                        device);
  if (!result) {
    return false;
  }

  return true;
}

void PBRModel::Shutdown() { ReleaseTextures(); }

void PBRModel::Render(const IShader &shader,
                      const ShaderParameterContainer &parameterContainer,
                      ID3D11DeviceContext *deviceContext) const {

  RenderBuffers(deviceContext);

  shader.Render(GetIndexCount(), parameterContainer, deviceContext);
}

ID3D11ShaderResourceView *PBRModel::GetTexture(int index) {
  return textures_[index].GetTexture();
}

bool PBRModel::InitializeBuffers(ID3D11Device *device) {

  // Check for overflow in buffer size calculation
  // Use (max)() to avoid Windows.h max macro conflict
  constexpr UINT MAX_BUFFER_SIZE = (std::numeric_limits<UINT>::max)();
  if (vertex_count_ > MAX_BUFFER_SIZE / sizeof(VertexType)) {
    std::cerr << "Error: PBR vertex count too large, would overflow buffer size calculation" << std::endl;
    return false;
  }
  if (index_count_ > MAX_BUFFER_SIZE / sizeof(unsigned long)) {
    std::cerr << "Error: PBR index count too large, would overflow buffer size calculation" << std::endl;
    return false;
  }

  D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
  D3D11_SUBRESOURCE_DATA vertexData, indexData;

  // Use std::vector for automatic memory management and exception safety
  std::vector<VertexType> vertices(vertex_count_);
  std::vector<unsigned long> indices(index_count_);

  for (int i = 0; i < vertex_count_; i++) {
    vertices[i].position = XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
    vertices[i].texture = XMFLOAT2(model_[i].tu, model_[i].tv);
    vertices[i].normal = XMFLOAT3(model_[i].nx, model_[i].ny, model_[i].nz);
    vertices[i].tangent = XMFLOAT3(model_[i].tx, model_[i].ty, model_[i].tz);
    vertices[i].binormal = XMFLOAT3(model_[i].bx, model_[i].by, model_[i].bz);

    indices[i] = i;
  }

  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexType) * vertex_count_);
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = 0;
  vertexBufferDesc.MiscFlags = 0;
  vertexBufferDesc.StructureByteStride = 0;

  vertexData.pSysMem = vertices.data();
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  auto result =
      device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertex_buffer_);
  if (FAILED(result)) {
    return false;
  }

  indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned long) * index_count_);
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  indexBufferDesc.CPUAccessFlags = 0;
  indexBufferDesc.MiscFlags = 0;
  indexBufferDesc.StructureByteStride = 0;

  indexData.pSysMem = indices.data();
  indexData.SysMemPitch = 0;
  indexData.SysMemSlicePitch = 0;

  result = device->CreateBuffer(&indexBufferDesc, &indexData, &index_buffer_);
  if (FAILED(result)) {
    return false;
  }

  // No need to manually delete - std::vector handles cleanup automatically
  // Memory is exception-safe and will be freed even if an exception is thrown

  return true;
}

void PBRModel::SetParameterCallback(ShaderParameterCallback callback) {
  // PBRModel doesn't use parameter callbacks by default
  // Derived classes can override if needed
}

ShaderParameterCallback PBRModel::GetParameterCallback() const {
  // Return empty callback - PBRModel uses default parameter behavior
  // This is safe and allows objects to render without custom parameter logic
  return nullptr;
}

void PBRModel::RenderBuffers(ID3D11DeviceContext *deviceContext) const {

  unsigned int stride = sizeof(VertexType);
  unsigned int offset = 0;

  deviceContext->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(),
                                    &stride, &offset);

  deviceContext->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool PBRModel::LoadTextures(const string &filename1, const string &filename2,
                            const string &filename3, ID3D11Device *device) {

  textures_ = std::make_unique<TGATexture[]>(3);

  auto result = textures_[0].Initialize(filename1.c_str(), device);
  if (!result) {
    return false;
  }

  result = textures_[1].Initialize(filename2.c_str(), device);
  if (!result) {
    return false;
  }

  result = textures_[2].Initialize(filename3.c_str(), device);
  if (!result) {
    return false;
  }

  return true;
}

void PBRModel::ReleaseTextures() { textures_.reset(); }

bool PBRModel::LoadModel(const char *filename) {
  ifstream fin;
  char input;

  fin.open(filename);

  // If it could not open the file then exit.
  if (fin.fail()) {
    return false;
  }

  // Add safety limit to prevent infinite loops on malformed files
  constexpr int MAX_PARSE_ITERATIONS = 10000;
  int iterations = 0;

  // Read up to the value of vertex count with bounds checking
  while (fin.get(input) && input != ':' && ++iterations < MAX_PARSE_ITERATIONS) {
    // Continue searching
  }

  if (iterations >= MAX_PARSE_ITERATIONS || !fin) {
    std::cerr << "Error: PBR model file format error - first delimiter not found: " 
              << filename << std::endl;
    return false;
  }

  // Read in the vertex count.
  fin >> vertex_count_;

  // Validate vertex count to prevent excessive memory allocation or overflow
  constexpr int MAX_VERTEX_COUNT = 1000000; // Reasonable maximum
  if (vertex_count_ <= 0 || vertex_count_ > MAX_VERTEX_COUNT) {
    std::cerr << "Error: Invalid vertex count (" << vertex_count_ 
              << ") in PBR model file: " << filename << std::endl;
    return false;
  }

  // Set the number of indices to be the same as the vertex count.
  index_count_ = vertex_count_;

  // Create the model using the vertex count that was read in.
  model_.resize(vertex_count_);

  // Read up to the beginning of the data with bounds checking
  iterations = 0;
  while (fin.get(input) && input != ':' && ++iterations < MAX_PARSE_ITERATIONS) {
    // Continue searching
  }

  if (iterations >= MAX_PARSE_ITERATIONS || !fin) {
    std::cerr << "Error: PBR model file format error - second delimiter not found: " 
              << filename << std::endl;
    return false;
  }

  fin.get(input);
  fin.get(input);

  // Read in the vertex data with stream state checking
  for (int i = 0; i < vertex_count_; i++) {
    fin >> model_[i].x >> model_[i].y >> model_[i].z;
    fin >> model_[i].tu >> model_[i].tv;
    fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;

    // Check for read errors after each vertex
    if (fin.fail()) {
      std::cerr << "Error: Failed to read vertex data at index " << i 
                << " in PBR model file: " << filename << std::endl;
      return false;
    }
  }

  // Close the model file.
  fin.close();

  return true;
}

void PBRModel::CalculateModelVectors() {

  TempVertexType vertex1, vertex2, vertex3;
  VectorType tangent, binormal;

  // Calculate the number of faces in the model.
  auto faceCount = vertex_count_ / 3;

  // Initialize the index to the model data.
  auto index = 0;

  // Go through all the faces and calculate the the tangent and binormal
  // vectors.
  for (int i = 0; i < faceCount; i++) {
    // Get the three vertices for this face from the model.
    vertex1.x = model_[index].x;
    vertex1.y = model_[index].y;
    vertex1.z = model_[index].z;
    vertex1.tu = model_[index].tu;
    vertex1.tv = model_[index].tv;
    index++;

    vertex2.x = model_[index].x;
    vertex2.y = model_[index].y;
    vertex2.z = model_[index].z;
    vertex2.tu = model_[index].tu;
    vertex2.tv = model_[index].tv;
    index++;

    vertex3.x = model_[index].x;
    vertex3.y = model_[index].y;
    vertex3.z = model_[index].z;
    vertex3.tu = model_[index].tu;
    vertex3.tv = model_[index].tv;
    index++;

    // Calculate the tangent and binormal of that face.
    CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

    // Store the tangent and binormal for this face back in the model structure.
    model_[index - 1].tx = tangent.x;
    model_[index - 1].ty = tangent.y;
    model_[index - 1].tz = tangent.z;
    model_[index - 1].bx = binormal.x;
    model_[index - 1].by = binormal.y;
    model_[index - 1].bz = binormal.z;

    model_[index - 2].tx = tangent.x;
    model_[index - 2].ty = tangent.y;
    model_[index - 2].tz = tangent.z;
    model_[index - 2].bx = binormal.x;
    model_[index - 2].by = binormal.y;
    model_[index - 2].bz = binormal.z;

    model_[index - 3].tx = tangent.x;
    model_[index - 3].ty = tangent.y;
    model_[index - 3].tz = tangent.z;
    model_[index - 3].bx = binormal.x;
    model_[index - 3].by = binormal.y;
    model_[index - 3].bz = binormal.z;
  }
}

void PBRModel::CalculateTangentBinormal(TempVertexType vertex1,
                                        TempVertexType vertex2,
                                        TempVertexType vertex3,
                                        VectorType &tangent,
                                        VectorType &binormal) {

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

  // Calculate the cross products and multiply by the coefficient to get the
  // tangent and binormal.
  tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
  tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
  tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

  binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
  binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
  binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

  // Calculate the length of this normal.
  auto length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) +
                     (tangent.z * tangent.z));

  // Normalize the normal and then store it
  tangent.x = tangent.x / length;
  tangent.y = tangent.y / length;
  tangent.z = tangent.z / length;

  // Calculate the length of this normal.
  length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) +
                (binormal.z * binormal.z));

  // Normalize the normal and then store it
  binormal.x = binormal.x / length;
  binormal.y = binormal.y / length;
  binormal.z = binormal.z / length;
}
