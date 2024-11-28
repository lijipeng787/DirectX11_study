#include "particlesystemclass.h"
#include "../CommonFramework/DirectX11Device.h"
#include "textureclass.h"

#include <DirectXMath.h>

using namespace DirectX;

struct ParticleType {
  float positionX, positionY, positionZ;
  float red, green, blue;
  float velocity;
  bool active;
};

struct VertexType {
  XMFLOAT3 position;
  XMFLOAT2 texture;
  XMFLOAT4 color;
};

bool ParticleSystemClass::Initialize(WCHAR *textureFilename) {

  // Load the texture that is used for the particles.
  auto result = LoadTexture(textureFilename);
  if (!result) {
    return false;
  }

  // Initialize the particle system.
  result = InitializeParticleSystem();
  if (!result) {
    return false;
  }

  // Create the buffers that will be used to render the particles with.
  result = InitializeBuffers();
  if (!result) {
    return false;
  }

  return true;
}

void ParticleSystemClass::Shutdown() {

  ShutdownBuffers();

  ShutdownParticleSystem();

  ReleaseTexture();
}

bool ParticleSystemClass::Frame(float frameTime) {

  KillParticles();

  // Emit new particles.
  EmitParticles(frameTime);

  // Update the position of the particles.
  UpdateParticles(frameTime);

  // Update the dynamic vertex buffer with the new position of each particle.
  auto result = UpdateBuffers();
  if (!result) {
    return false;
  }

  return true;
}

void ParticleSystemClass::Render() { RenderBuffers(); }

ID3D11ShaderResourceView *ParticleSystemClass::GetTexture() {
  return texture_->GetTexture();
}

int ParticleSystemClass::GetIndexCount() { return index_count_; }

bool ParticleSystemClass::LoadTexture(WCHAR *filename) {

  texture_ = new TextureClass();
  if (!texture_) {
    return false;
  }

  auto result = texture_->Initialize(filename);
  if (!result) {
    return false;
  }

  return true;
}

void ParticleSystemClass::ReleaseTexture() {

  if (texture_) {
    texture_->Shutdown();
    delete texture_;
    texture_ = nullptr;
  }
}

bool ParticleSystemClass::InitializeParticleSystem() {

  // Set the random deviation of where the particles can be located when
  // emitted.
  particle_deviatio_x_ = 0.5f;
  particle_deviation_y_ = 0.1f;
  particle_deviation_z_ = 2.0f;

  // Set the speed and speed variation of particles.
  particle_velocity_ = 1.0f;
  particle_velocity_variation_ = 0.2f;

  // Set the physical size of the particles.
  particle_size_ = 0.2f;

  // Set the number of particles to emit per second.
  particles_per_second_ = 250.0f;

  // Set the maximum number of particles allowed in the particle system.
  max_particles_ = 5000;

  // Create the particle list.
  particle_list_ = new ParticleType[max_particles_];
  if (!particle_list_) {
    return false;
  }

  // Initialize the particle list.
  for (int i = 0; i < max_particles_; i++) {
    particle_list_[i].active = false;
  }

  // Initialize the current particle count to zero since none are emitted yet.
  current_particle_count_ = 0;

  // Clear the initial accumulated time for the particle per second emission
  // rate.
  accumulated_time_ = 0.0f;

  return true;
}

void ParticleSystemClass::ShutdownParticleSystem() {

  if (particle_list_) {
    delete[] particle_list_;
    particle_list_ = 0;
  }
}

bool ParticleSystemClass::InitializeBuffers() {

  // Set the maximum number of vertices in the vertex array.
  vertex_count_ = max_particles_ * 6;

  // Set the maximum number of indices in the index array.
  index_count_ = vertex_count_;

  vertices_ = new VertexType[vertex_count_];
  if (!vertices_) {
    return false;
  }

  auto indices = new unsigned long[index_count_];
  if (!indices) {
    return false;
  }

  memset(vertices_, 0, (sizeof(VertexType) * vertex_count_));

  for (int i = 0; i < index_count_; i++) {
    indices[i] = i;
  }

  D3D11_BUFFER_DESC vertex_buffer_desc;

  vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
  vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  vertex_buffer_desc.MiscFlags = 0;
  vertex_buffer_desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA vertex_data;

  vertex_data.pSysMem = vertices_;
  vertex_data.SysMemPitch = 0;
  vertex_data.SysMemSlicePitch = 0;

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  auto result =
      device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC index_buffer_desc;

  index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
  index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  index_buffer_desc.CPUAccessFlags = 0;
  index_buffer_desc.MiscFlags = 0;
  index_buffer_desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA indexData;

  indexData.pSysMem = indices;
  indexData.SysMemPitch = 0;
  indexData.SysMemSlicePitch = 0;

  result = device->CreateBuffer(&index_buffer_desc, &indexData, &index_buffer_);
  if (FAILED(result)) {
    return false;
  }

  delete[] indices;
  indices = 0;

  return true;
}

void ParticleSystemClass::ShutdownBuffers() {

  if (index_buffer_) {
    index_buffer_->Release();
    index_buffer_ = nullptr;
  }

  if (vertex_buffer_) {
    vertex_buffer_->Release();
    vertex_buffer_ = nullptr;
  }
}

void ParticleSystemClass::EmitParticles(float frameTime) {

  // Increment the frame time.
  accumulated_time_ += frameTime;

  // Set emit particle to false for now.
  auto emitParticle = false, found = false;

  // Check if it is time to emit a new particle or not.
  if (accumulated_time_ > (1000.0f / particles_per_second_)) {
    accumulated_time_ = 0.0f;
    emitParticle = true;
  }

  float positionX, positionY, positionZ, velocity, red, green, blue;
  int index, i, j;

  // If there are particles to emit then emit one per frame.
  if ((emitParticle == true) &&
      (current_particle_count_ < (max_particles_ - 1))) {

    current_particle_count_++;

    // Now generate the randomized particle properties.
    positionX =
        (((float)rand() - (float)rand()) / RAND_MAX) * particle_deviatio_x_;
    positionY =
        (((float)rand() - (float)rand()) / RAND_MAX) * particle_deviation_y_;
    positionZ =
        (((float)rand() - (float)rand()) / RAND_MAX) * particle_deviation_z_;

    velocity =
        particle_velocity_ + (((float)rand() - (float)rand()) / RAND_MAX) *
                                 particle_velocity_variation_;

    red = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
    green = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;
    blue = (((float)rand() - (float)rand()) / RAND_MAX) + 0.5f;

    // Now since the particles need to be rendered from back to front for
    // blending we have to sort the particle array. We will sort using Z depth
    // so we need to find where in the list the particle should be inserted.
    index = 0;
    found = false;
    while (!found) {
      if ((particle_list_[index].active == false) ||
          (particle_list_[index].positionZ < positionZ)) {
        found = true;
      } else {
        index++;
      }
    }

    // Now that we know the location to insert into we need to copy the array
    // over by one position from the index to make room for the new particle.
    i = current_particle_count_;
    j = i - 1;

    while (i != index) {
      particle_list_[i].positionX = particle_list_[j].positionX;
      particle_list_[i].positionY = particle_list_[j].positionY;
      particle_list_[i].positionZ = particle_list_[j].positionZ;
      particle_list_[i].red = particle_list_[j].red;
      particle_list_[i].green = particle_list_[j].green;
      particle_list_[i].blue = particle_list_[j].blue;
      particle_list_[i].velocity = particle_list_[j].velocity;
      particle_list_[i].active = particle_list_[j].active;
      i--;
      j--;
    }

    // Now insert it into the particle array in the correct depth order.
    particle_list_[index].positionX = positionX;
    particle_list_[index].positionY = positionY;
    particle_list_[index].positionZ = positionZ;
    particle_list_[index].red = red;
    particle_list_[index].green = green;
    particle_list_[index].blue = blue;
    particle_list_[index].velocity = velocity;
    particle_list_[index].active = true;
  }
}

void ParticleSystemClass::UpdateParticles(float frameTime) {

  // Each frame we update all the particles by making them move downwards using
  // their position, velocity, and the frame time.
  for (int i = 0; i < current_particle_count_; i++) {
    particle_list_[i].positionY =
        particle_list_[i].positionY -
        (particle_list_[i].velocity * frameTime * 0.001f);
  }
}

void ParticleSystemClass::KillParticles() {

  int j = 0;

  // Kill all the particles that have gone below a certain height range.
  for (int i = 0; i < max_particles_; i++) {
    if ((particle_list_[i].active == true) &&
        (particle_list_[i].positionY < -3.0f)) {
      particle_list_[i].active = false;
      current_particle_count_--;

      // Now shift all the live particles back up the array to erase the
      // destroyed particle and keep the array sorted correctly.
      for (j = i; j < max_particles_ - 1; j++) {
        particle_list_[j].positionX = particle_list_[j + 1].positionX;
        particle_list_[j].positionY = particle_list_[j + 1].positionY;
        particle_list_[j].positionZ = particle_list_[j + 1].positionZ;
        particle_list_[j].red = particle_list_[j + 1].red;
        particle_list_[j].green = particle_list_[j + 1].green;
        particle_list_[j].blue = particle_list_[j + 1].blue;
        particle_list_[j].velocity = particle_list_[j + 1].velocity;
        particle_list_[j].active = particle_list_[j + 1].active;
      }
    }
  }
}

bool ParticleSystemClass::UpdateBuffers() {

  D3D11_MAPPED_SUBRESOURCE mappedResource;

  memset(vertices_, 0, (sizeof(VertexType) * vertex_count_));

  // Now build the vertex array from the particle list array.  Each particle is
  // a quad made out of two triangles.
  int index = 0;

  for (int i = 0; i < current_particle_count_; i++) {
    // Bottom left.
    vertices_[index].position =
        XMFLOAT3(particle_list_[i].positionX - particle_size_,
                 particle_list_[i].positionY - particle_size_,
                 particle_list_[i].positionZ);
    vertices_[index].texture = XMFLOAT2(0.0f, 1.0f);
    vertices_[index].color =
        XMFLOAT4(particle_list_[i].red, particle_list_[i].green,
                 particle_list_[i].blue, 1.0f);
    index++;

    // Top left.
    vertices_[index].position =
        XMFLOAT3(particle_list_[i].positionX - particle_size_,
                 particle_list_[i].positionY + particle_size_,
                 particle_list_[i].positionZ);
    vertices_[index].texture = XMFLOAT2(0.0f, 0.0f);
    vertices_[index].color =
        XMFLOAT4(particle_list_[i].red, particle_list_[i].green,
                 particle_list_[i].blue, 1.0f);
    index++;

    // Bottom right.
    vertices_[index].position =
        XMFLOAT3(particle_list_[i].positionX + particle_size_,
                 particle_list_[i].positionY - particle_size_,
                 particle_list_[i].positionZ);
    vertices_[index].texture = XMFLOAT2(1.0f, 1.0f);
    vertices_[index].color =
        XMFLOAT4(particle_list_[i].red, particle_list_[i].green,
                 particle_list_[i].blue, 1.0f);
    index++;

    // Bottom right.
    vertices_[index].position =
        XMFLOAT3(particle_list_[i].positionX + particle_size_,
                 particle_list_[i].positionY - particle_size_,
                 particle_list_[i].positionZ);
    vertices_[index].texture = XMFLOAT2(1.0f, 1.0f);
    vertices_[index].color =
        XMFLOAT4(particle_list_[i].red, particle_list_[i].green,
                 particle_list_[i].blue, 1.0f);
    index++;

    // Top left.
    vertices_[index].position =
        XMFLOAT3(particle_list_[i].positionX - particle_size_,
                 particle_list_[i].positionY + particle_size_,
                 particle_list_[i].positionZ);
    vertices_[index].texture = XMFLOAT2(0.0f, 0.0f);
    vertices_[index].color =
        XMFLOAT4(particle_list_[i].red, particle_list_[i].green,
                 particle_list_[i].blue, 1.0f);
    index++;

    // Top right.
    vertices_[index].position =
        XMFLOAT3(particle_list_[i].positionX + particle_size_,
                 particle_list_[i].positionY + particle_size_,
                 particle_list_[i].positionZ);
    vertices_[index].texture = XMFLOAT2(1.0f, 0.0f);
    vertices_[index].color =
        XMFLOAT4(particle_list_[i].red, particle_list_[i].green,
                 particle_list_[i].blue, 1.0f);
    index++;
  }

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  auto result = device_context->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD,
                                    0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  VertexType *verticesPtr;

  verticesPtr = (VertexType *)mappedResource.pData;

  memcpy(verticesPtr, (void *)vertices_, (sizeof(VertexType) * vertex_count_));

  device_context->Unmap(vertex_buffer_, 0);

  return true;
}

void ParticleSystemClass::RenderBuffers() {

  unsigned int stride = sizeof(VertexType);
  unsigned int offset = 0;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

  device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

  // Set the type of primitive that should be rendered from this vertex buffer.
  device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}