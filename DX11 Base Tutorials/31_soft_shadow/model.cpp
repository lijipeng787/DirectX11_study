#include "model.h"

#include "../CommonFramework2/DirectX11Device.h"
#include "IShader.h"
#include "ShaderParameterContainer.h"

#include <DirectXMath.h>
#include <fstream>

using namespace std;
using namespace DirectX;

Model::~Model() { Shutdown(); }

bool Model::Initialize(const std::string &modelFilename,
                            const std::wstring &textureFilename) {
  if (!LoadModel(modelFilename) || !InitializeBuffers() ||
      !LoadTexture(textureFilename)) {
    return false;
  }
  return true;
}

void Model::Shutdown() {
  ShutdownBuffers();
  ReleaseModel();
}

void Model::Render(
    const IShader &shader,
    const ShaderParameterContainer &parameterContainer) const {

  RenderBuffers();

  shader.Render(GetIndexCount(), parameterContainer);
}

void Model::SetParameterCallback(ShaderParameterCallback callback) {}

ShaderParameterCallback Model::GetParameterCallback() const {
  return [this](ShaderParameterContainer &params) { assert(0); };
}

ID3D11ShaderResourceView *Model::GetTexture() const {
  return texture_ ? texture_->GetTexture() : nullptr;
}

bool Model::InitializeBuffers() {
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
  vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertex_count_;
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vertexData = {};
  vertexData.pSysMem = vertices.data();

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  HRESULT result =
      device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertex_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC indexBufferDesc = {};
  indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  indexBufferDesc.ByteWidth = sizeof(unsigned long) * index_count_;
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

void Model::RenderBuffers() const {

  UINT stride = sizeof(Vertex);
  UINT offset = 0;

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  deviceContext->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(),
                                    &stride, &offset);
  deviceContext->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::LoadTexture(const std::wstring &filename) {
  texture_ = std::make_unique<Texture>();
  return texture_->Initialize(filename.c_str());
}

bool Model::LoadModel(const std::string &filename) {
  std::ifstream fin(filename);
  if (!fin) {
    return false;
  }

  char input;
  fin.get(input);
  while (input != ':') {
    fin.get(input);
  }

  fin >> vertex_count_;
  index_count_ = vertex_count_;
  model_.resize(vertex_count_);

  fin.get(input);
  while (input != ':') {
    fin.get(input);
  }
  fin.get(input);
  fin.get(input);

  for (int i = 0; i < vertex_count_; i++) {
    fin >> model_[i].x >> model_[i].y >> model_[i].z;
    fin >> model_[i].tu >> model_[i].tv;
    fin >> model_[i].nx >> model_[i].ny >> model_[i].nz;
  }

  fin.close();
  return true;
}

void Model::ReleaseModel() { model_.clear(); }

bool PBRModel::Initialize(const char *modelFilename,
                               const string &textureFilename1,
                               const string &textureFilename2,
                               const string &textureFilename3) {

  auto result = LoadModel(modelFilename);
  if (!result) {
    return false;
  }

  // Calculate the tangent and binormal vectors for the model.
  CalculateModelVectors();

  result = InitializeBuffers();
  if (!result) {
    return false;
  }

  result = LoadTextures(textureFilename1, textureFilename2, textureFilename3);
  if (!result) {
    return false;
  }

  return true;
}

void PBRModel::Shutdown() { ReleaseTextures(); }

void PBRModel::Render(
    const IShader &shader,
    const ShaderParameterContainer &parameterContainer) const {

  RenderBuffers();

  shader.Render(GetIndexCount(), parameterContainer);
}

ID3D11ShaderResourceView *PBRModel::GetTexture(int index) {
  return m_Textures[index].GetTexture();
}

bool PBRModel::InitializeBuffers() {

  D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
  D3D11_SUBRESOURCE_DATA vertexData, indexData;

  auto vertices = new VertexType[m_vertexCount];

  auto indices = new unsigned long[m_indexCount];

  for (int i = 0; i < m_vertexCount; i++) {
    vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
    vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
    vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);
    vertices[i].tangent = XMFLOAT3(m_model[i].tx, m_model[i].ty, m_model[i].tz);
    vertices[i].binormal =
        XMFLOAT3(m_model[i].bx, m_model[i].by, m_model[i].bz);

    indices[i] = i;
  }

  vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
  vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags = 0;
  vertexBufferDesc.MiscFlags = 0;
  vertexBufferDesc.StructureByteStride = 0;

  vertexData.pSysMem = vertices;
  vertexData.SysMemPitch = 0;
  vertexData.SysMemSlicePitch = 0;

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  auto result =
      device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
  if (FAILED(result)) {
    return false;
  }

  indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  indexBufferDesc.CPUAccessFlags = 0;
  indexBufferDesc.MiscFlags = 0;
  indexBufferDesc.StructureByteStride = 0;

  indexData.pSysMem = indices;
  indexData.SysMemPitch = 0;
  indexData.SysMemSlicePitch = 0;

  result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
  if (FAILED(result)) {
    return false;
  }

  delete[] vertices;
  vertices = 0;

  delete[] indices;
  indices = 0;

  return true;
}

void PBRModel::SetParameterCallback(ShaderParameterCallback callback) {}

ShaderParameterCallback PBRModel::GetParameterCallback() const {
  return [this](ShaderParameterContainer &params) { assert(0); };
}

void PBRModel::RenderBuffers() const {

  unsigned int stride = sizeof(VertexType);
  unsigned int offset = 0;

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),
                                    &stride, &offset);

  deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool PBRModel::LoadTextures(const string &filename1,
                                 const string &filename2,
                                 const string &filename3) {

  m_Textures = new TGATexture[3];

  auto result = m_Textures[0].Initialize(filename1.c_str());
  if (!result) {
    return false;
  }

  result = m_Textures[1].Initialize(filename2.c_str());
  if (!result) {
    return false;
  }

  result = m_Textures[2].Initialize(filename3.c_str());
  if (!result) {
    return false;
  }

  return true;
}

void PBRModel::ReleaseTextures() {
  if (m_Textures) {
    delete[] m_Textures;
    m_Textures = nullptr;
  }
}

bool PBRModel::LoadModel(const char *filename) {
  ifstream fin;
  char input;

  fin.open(filename);

  // If it could not open the file then exit.
  if (fin.fail()) {
    return false;
  }

  // Read up to the value of vertex count.
  fin.get(input);
  while (input != ':') {
    fin.get(input);
  }

  // Read in the vertex count.
  fin >> m_vertexCount;

  // Set the number of indices to be the same as the vertex count.
  m_indexCount = m_vertexCount;

  // Create the model using the vertex count that was read in.
  m_model.resize(m_vertexCount);

  // Read up to the beginning of the data.
  fin.get(input);
  while (input != ':') {
    fin.get(input);
  }
  fin.get(input);
  fin.get(input);

  // Read in the vertex data.
  for (int i = 0; i < m_vertexCount; i++) {
    fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
    fin >> m_model[i].tu >> m_model[i].tv;
    fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
  }

  // Close the model file.
  fin.close();

  return true;
}

void PBRModel::CalculateModelVectors() {

  TempVertexType vertex1, vertex2, vertex3;
  VectorType tangent, binormal;

  // Calculate the number of faces in the model.
  auto faceCount = m_vertexCount / 3;

  // Initialize the index to the model data.
  auto index = 0;

  // Go through all the faces and calculate the the tangent and binormal
  // vectors.
  for (int i = 0; i < faceCount; i++) {
    // Get the three vertices for this face from the model.
    vertex1.x = m_model[index].x;
    vertex1.y = m_model[index].y;
    vertex1.z = m_model[index].z;
    vertex1.tu = m_model[index].tu;
    vertex1.tv = m_model[index].tv;
    index++;

    vertex2.x = m_model[index].x;
    vertex2.y = m_model[index].y;
    vertex2.z = m_model[index].z;
    vertex2.tu = m_model[index].tu;
    vertex2.tv = m_model[index].tv;
    index++;

    vertex3.x = m_model[index].x;
    vertex3.y = m_model[index].y;
    vertex3.z = m_model[index].z;
    vertex3.tu = m_model[index].tu;
    vertex3.tv = m_model[index].tv;
    index++;

    // Calculate the tangent and binormal of that face.
    CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);

    // Store the tangent and binormal for this face back in the model structure.
    m_model[index - 1].tx = tangent.x;
    m_model[index - 1].ty = tangent.y;
    m_model[index - 1].tz = tangent.z;
    m_model[index - 1].bx = binormal.x;
    m_model[index - 1].by = binormal.y;
    m_model[index - 1].bz = binormal.z;

    m_model[index - 2].tx = tangent.x;
    m_model[index - 2].ty = tangent.y;
    m_model[index - 2].tz = tangent.z;
    m_model[index - 2].bx = binormal.x;
    m_model[index - 2].by = binormal.y;
    m_model[index - 2].bz = binormal.z;

    m_model[index - 3].tx = tangent.x;
    m_model[index - 3].ty = tangent.y;
    m_model[index - 3].tz = tangent.z;
    m_model[index - 3].bx = binormal.x;
    m_model[index - 3].by = binormal.y;
    m_model[index - 3].bz = binormal.z;
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
