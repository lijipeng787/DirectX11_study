#pragma once

#include <d3d11.h>

struct ParticleType;
struct VertexType;

struct TextureClass;

class ParticleSystemClass {
public:
  ParticleSystemClass() {}

  ParticleSystemClass(const ParticleSystemClass &rhs) = delete;

  ~ParticleSystemClass() {}

public:
  bool Initialize(WCHAR *);

  void Shutdown();

  bool Frame(float);

  void Render();

  ID3D11ShaderResourceView *GetTexture();

  int GetIndexCount();

private:
  bool LoadTexture(WCHAR *);

  void ReleaseTexture();

  bool InitializeParticleSystem();

  void ShutdownParticleSystem();

  bool InitializeBuffers();

  void ShutdownBuffers();

  void EmitParticles(float);

  void UpdateParticles(float);

  void KillParticles();

  bool UpdateBuffers();

  void RenderBuffers();

private:
  float particle_deviatio_x_, particle_deviation_y_, particle_deviation_z_;

  float particle_velocity_, particle_velocity_variation_;

  float particle_size_, particles_per_second_;

  int max_particles_;

  int current_particle_count_;

  float accumulated_time_;

  TextureClass *texture_;

  ParticleType *particle_list_;

  int vertex_count_, index_count_;

  VertexType *vertices_;

  ID3D11Buffer *vertex_buffer_, *index_buffer_;
};
