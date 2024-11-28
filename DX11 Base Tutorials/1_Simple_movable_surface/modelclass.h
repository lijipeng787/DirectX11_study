#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include "d3dclass.h"
#include "textureclass.h"
#include <DirectXMath.h>
#include <memory>

class Bitmap {
private:
  struct VertexType {
    DirectX::XMFLOAT3 position_;
    DirectX::XMFLOAT2 texture_;
  };

  struct MatrixBufferType {
    DirectX::XMMATRIX world_;
    DirectX::XMMATRIX view_;
    DirectX::XMMATRIX orthogonality_;
    DirectX::XMMATRIX padding_;
  };

public:
  Bitmap();

  Bitmap(const Bitmap &);

  ~Bitmap();

public:
  bool Initialize(WCHAR *texture_filename, UINT screen_width,
                  UINT screen_height, UINT bitmap_width, UINT bitmap_height);

  void SetVertexShader(const D3D12_SHADER_BYTECODE &verte_shader_bitecode);

  void SetPixelShader(const D3D12_SHADER_BYTECODE &pixel_shader_bitcode);

  const D3D12_VERTEX_BUFFER_VIEW &GetVertexBufferView() const;

  const D3D12_INDEX_BUFFER_VIEW &GetIndexBufferView() const;

  RootSignaturePtr GetRootSignature() const;

  PipelineStateObjectPtr GetPipelineStateObject() const;

  DescriptorHeapPtr GetShaderRescourceView() const;

  D3d12ResourcePtr GetConstantBuffer() const;

  bool UpdateConstantBuffer(DirectX::XMMATRIX &world, DirectX::XMMATRIX &view,
                            DirectX::XMMATRIX &orthogonality);

  bool UpdateBitmapPos(int pos_x, int pos_y);

  int GetIndexCount();

private:
  bool InitializeBuffers();

  bool InitializeGraphicsPipelineState();

  bool InitializeRootSignature();

  bool LoadTexture(WCHAR *texture_filename);

private:
  D3D12_SHADER_BYTECODE vertex_shader_bitecode_ = {};

  D3D12_SHADER_BYTECODE pixel_shader_bitcode_ = {};

  RootSignaturePtr root_signature_ = nullptr;

  PipelineStateObjectPtr pso_ = nullptr;

  PipelineStateObjectPtr pso_depth_disabled_ = nullptr;

  D3d12ResourcePtr vertex_buffer_ = nullptr;

  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;

  D3d12ResourcePtr index_buffer_ = nullptr;

  D3D12_INDEX_BUFFER_VIEW index_buffer_view_;

  D3d12ResourcePtr constant_buffer_ = nullptr;

  MatrixBufferType matrix_constant_data_ = {};

  UINT index_count_ = 0;

  UINT vertex_count_ = 0;

  UINT screen_width_ = 0;

  UINT screen_height_ = 0;

  UINT bitmap_height_ = 0;

  UINT bitmap_width_ = 0;

  int previous_pos_x_ = -1;

  int previous_pos_y_ = -1;

  std::shared_ptr<Texture> texture_ = nullptr;
};

#endif