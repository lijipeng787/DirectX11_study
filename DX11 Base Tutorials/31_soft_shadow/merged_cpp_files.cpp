

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\depthshaderclass.h
#ifndef _DEPTHSHADERCLASS_H_
#define _DEPTHSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class DepthShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  DepthShaderClass() = default;

  DepthShaderClass(const DepthShaderClass &) = delete;

  ~DepthShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\GameObject.h
#pragma once

#include "modelclass.h"
#include <DirectXMath.h>
#include <memory>

class GameObject {
public:
  GameObject(std::shared_ptr<ModelClass> model);

public:
  void SetPosition(const DirectX::XMFLOAT3 &position);

  void SetRotation(const DirectX::XMFLOAT3 &rotation);

  void SetScale(const DirectX::XMFLOAT3 &scale);

  void Update(float deltaTime);

  DirectX::XMMATRIX GetWorldMatrix() const;

  std::shared_ptr<ModelClass> GetModel() const;

private:
  std::shared_ptr<ModelClass> model_;
  DirectX::XMFLOAT3 position_;
  DirectX::XMFLOAT3 rotation_;
  DirectX::XMFLOAT3 scale_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\Graphics.h
#pragma once

#include "../CommonFramework/GraphicsBase.h"

const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;

class DirectX11Device;
class Camera;
class ModelClass;
class DepthShaderClass;
class ShadowShaderClass;
class LightClass;
class RenderTextureClass;
class OrthoWindowClass;
class HorizontalBlurShaderClass;
class VerticalBlurShaderClass;
class SoftShadowShaderClass;
class TextureShaderClass;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass();

  GraphicsClass(const GraphicsClass &rhs) = delete;

  GraphicsClass &operator=(const GraphicsClass &rhs) = delete;

  virtual ~GraphicsClass();

public:
  virtual bool Initialize(int, int, HWND) override;

  virtual void Shutdown() override;

  virtual void Frame(float deltatime) override;

  virtual void Render() override;

public:
  void SetPosition(float x, float y, float z) {
    pos_x_ = x;
    pos_y_ = y;
    pos_z_ = z;
  }

  void SetRotation(float x, float y, float z) {
    rot_x_ = x;
    rot_y_ = y;
    rot_z_ = z;
  }

private:
  bool RenderSceneToTexture();

  bool RenderBlackAndWhiteShadows();

  bool DownSampleTexture();

  bool RenderHorizontalBlurToTexture();

  bool RenderVerticalBlurToTexture();

  bool UpSampleTexture();

private:
  float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

  float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

  Camera *camera_ = nullptr;

  ModelClass *m_CubeModel, *m_GroundModel, *m_SphereModel;

  LightClass *light_;

  RenderTextureClass *render_texture_, *m_BlackWhiteRenderTexture,
      *m_DownSampleTexure;

  RenderTextureClass *m_HorizontalBlurTexture, *m_VerticalBlurTexture,
      *m_UpSampleTexure;

  DepthShaderClass *depth_shader_;

  ShadowShaderClass *m_ShadowShader;

  OrthoWindowClass *m_SmallWindow, *m_FullScreenWindow;

  TextureShaderClass *texture_shader_;

  HorizontalBlurShaderClass *m_HorizontalBlurShader;

  VerticalBlurShaderClass *m_VerticalBlurShader;

  SoftShadowShaderClass *m_SoftShadowShader;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\horizontalblurshaderclass.h

#ifndef _HORIZONTALBLURSHADERCLASS_H_
#define _HORIZONTALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class HorizontalBlurShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenWidth;
    DirectX::XMFLOAT3 padding;
  };

public:
  HorizontalBlurShaderClass() = default;

  HorizontalBlurShaderClass(const HorizontalBlurShaderClass &) = delete;

  ~HorizontalBlurShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *, float) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> screen_size_buffer_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\IRenderable.h
#pragma once

#include <DirectXMath.h>

#include "ShaderParameterContainer.h"

class IShader;

class IRenderable {
public:
  virtual ~IRenderable() = default;

  virtual void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const = 0;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\IRenderGroup.h
#pragma once

#include "IRenderable.h"
#include <memory>
#include <vector>

class IRenderGroup {
public:
  virtual ~IRenderGroup() = default;

public:
  virtual void PreRender() = 0;

  virtual void PostRender() = 0;

  virtual const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const = 0;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\IShader.h
#pragma once

#include <Windows.h>

#include "ShaderParameterContainer.h"

class IShader {
public:
  virtual ~IShader() = default;

  virtual bool Initialize(HWND hwnd) = 0;

  virtual void Shutdown() = 0;

  virtual bool Render(int indexCount,
                      const ShaderParameterContainer &parameters) const = 0;

  // virtual const ShaderParameterLayout& GetParameterLayout() const = 0;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\lightclass.h
#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class LightClass {
public:
  LightClass();
  LightClass(const LightClass &);
  ~LightClass();

  void SetAmbientColor(float, float, float, float);
  void SetDiffuseColor(float, float, float, float);
  void SetPosition(float, float, float);
  void SetLookAt(float, float, float);

  XMFLOAT4 GetAmbientColor();
  XMFLOAT4 GetDiffuseColor();
  XMFLOAT3 GetPosition();

  void GenerateViewMatrix();
  void GenerateProjectionMatrix(float, float);

  void GetViewMatrix(XMMATRIX &);
  void GetProjectionMatrix(XMMATRIX &);

private:
  XMFLOAT4 ambient_color_;
  XMFLOAT4 diffuse_color_;
  XMFLOAT3 light_position_;
  XMFLOAT3 light_look_at_;
  XMMATRIX light_viewMatrix_;
  XMMATRIX projection_matrix_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\modelclass.h

#pragma once

#include "IRenderable.h"
#include "ShaderParameterLayout.h"
#include "textureclass.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <vector>

class ModelClass : public IRenderable {
public:
  ModelClass() = default;

  ModelClass(const ModelClass &) = delete;

  ModelClass &operator=(const ModelClass &) = delete;

  ~ModelClass();

public:
  bool Initialize(const std::string &modelFilename,
                  const std::wstring &textureFilename);

  void Shutdown();

  void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const override;

  int GetIndexCount() const { return index_count_; }

  ID3D11ShaderResourceView *GetTexture() const;

  DirectX::XMMATRIX GetWorldMatrix() const { return world_matrix_; }

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) {
    world_matrix_ = worldMatrix;
  }

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers() const;

  bool LoadTexture(const std::wstring &filename);

  void ReleaseTexture();

  bool LoadModel(const std::string &filename);

  void ReleaseModel();

private:
  struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
    DirectX::XMFLOAT3 normal;
  };

  struct ModelType {
    float x, y, z;
    float tu, tv;
    float nx, ny, nz;
  };

  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;

  int vertex_count_ = 0;
  int index_count_ = 0;

  std::unique_ptr<TextureClass> texture_;

  std::vector<ModelType> model_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\orthowindowclass.h

#ifndef _ORTHOWINDOWCLASS_H_
#define _ORTHOWINDOWCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

class OrthoWindowClass {
private:
  struct VertexType {
    XMFLOAT3 position;
    XMFLOAT2 texture;
  };

public:
  OrthoWindowClass();
  OrthoWindowClass(const OrthoWindowClass &);
  ~OrthoWindowClass();

  bool Initialize(int, int);
  void Shutdown();
  void Render(ID3D11DeviceContext *);

  int GetIndexCount();

private:
  bool InitializeBuffers(int, int);
  void ShutdownBuffers();
  void RenderBuffers(ID3D11DeviceContext *);

private:
  ID3D11Buffer *vertex_buffer_, *index_buffer_;
  int vertex_count_, index_count_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\positionclass.h
#pragma once

#include <math.h>

class PositionClass {
public:
  PositionClass();
  PositionClass(const PositionClass &);
  ~PositionClass();

  void SetPosition(float, float, float);
  void SetRotation(float, float, float);

  void GetPosition(float &, float &, float &);
  void GetRotation(float &, float &, float &);

  void SetFrameTime(float);

  void MoveForward(bool);
  void MoveBackward(bool);
  void MoveUpward(bool);
  void MoveDownward(bool);
  void TurnLeft(bool);
  void TurnRight(bool);
  void LookUpward(bool);
  void LookDownward(bool);

private:
  float position_x_, position_y_, position_z_;
  float rotation_x_, rotation_y_, rotation_z_;

  float frame_time_;

  float m_forwardSpeed, m_backwardSpeed;
  float m_upwardSpeed, m_downwardSpeed;
  float left_turning_speed_, right_turning_speed_;
  float m_lookUpSpeed, m_lookDownSpeed;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\RenderPipeline.h
#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "IShader.h"
#include "Scene.h"
#include "ShaderParameterLayout.h"

class RenderPipeline {
public:
  bool Initialize(HWND hwnd);

  void Shutdown();

  bool Render(const Scene &scene) const;

  void AddShader(const std::string &name, std::unique_ptr<IShader> shader);

  void RemoveShader(const std::string &name);

  IShader *GetShader(const std::string &name) const;

  void SetActiveShader(const std::string &name);

private:
  std::unordered_map<std::string, std::unique_ptr<IShader>> shaders_;
  std::unordered_map<std::string, ShaderParameterLayout> shader_layouts_;
  std::string active_shader_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\RenderSystem.h
#pragma once

#include <memory>

#include "RenderPipeline.h"

class RenderSystem {
public:
  bool Initialize(HWND hwnd);

  void Shutdown();

  void Render(const Scene &scene);

private:
  std::unique_ptr<RenderPipeline> pipeline_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\rendertextureclass.h
#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

class RenderTextureClass {
public:
  RenderTextureClass();
  RenderTextureClass(const RenderTextureClass &);
  ~RenderTextureClass();

  bool Initialize(int, int, float, float);
  void Shutdown();

  void SetRenderTarget(ID3D11DeviceContext *);
  void ClearRenderTarget(float, float, float, float);
  ID3D11ShaderResourceView *GetShaderResourceView();
  void GetProjectionMatrix(XMMATRIX &);
  void GetOrthoMatrix(XMMATRIX &);

private:
  ID3D11Texture2D *render_target_texture_;
  ID3D11RenderTargetView *render_target_view_;
  ID3D11ShaderResourceView *shader_resource_view_;
  ID3D11Texture2D *depth_stencil_buffer_;
  ID3D11DepthStencilView *depth_stencil_view_;
  D3D11_VIEWPORT viewport_;
  XMMATRIX projection_matrix_;
  XMMATRIX ortho_matrix_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\Scene.h Scene.h
#pragma once
#include <memory>
#include <vector>

#include "../CommonFramework/Camera.h"
#include "IRenderGroup.h"
#include "lightclass.h"

class Scene {
public:
  void AddRenderGroup(std::shared_ptr<IRenderGroup> renderGroup);

  void SetCamera(std::shared_ptr<Camera> camera);

  void AddLight(std::shared_ptr<LightClass> light);

  const std::vector<std::shared_ptr<IRenderGroup>> &GetRenderGroups() const {
    return renderGroups_;
  }

  std::shared_ptr<Camera> GetCamera() const { return camera_; }

  const std::vector<std::shared_ptr<LightClass>> &GetLights() const {
    return lights_;
  }

private:
  std::vector<std::shared_ptr<IRenderGroup>> renderGroups_;
  std::shared_ptr<Camera> camera_;
  std::vector<std::shared_ptr<LightClass>> lights_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\ShaderParameterContainer.h
#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include <any>
#include <string>
#include <unordered_map>

class ShaderParameterContainer {
public:
  void SetFloat(const std::string &name, float f);

  void SetMatrix(const std::string &name, const DirectX::XMMATRIX &matrix);

  void SetVector3(const std::string &name, const DirectX::XMFLOAT3 &vector);

  void SetVector4(const std::string &name, const DirectX::XMFLOAT4 &vector);

  void SetTexture(const std::string &name, ID3D11ShaderResourceView *texture);

  template <typename T> T Get(const std::string &name) const {
    auto it = parameters_.find(name);
    if (it == parameters_.end()) {
      throw std::runtime_error("Parameter not found: " + name);
    }
    try {
      return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast &) {
      throw std::runtime_error("Type mismatch for parameter: " + name);
    }
  }

  float GetFloat(const std::string &name) const { return Get<float>(name); }

  DirectX::XMMATRIX GetMatrix(const std::string &name) const {
    return Get<DirectX::XMMATRIX>(name);
  }

  DirectX::XMFLOAT3 GetVector3(const std::string &name) const {
    return Get<DirectX::XMFLOAT3>(name);
  }

  DirectX::XMFLOAT4 GetVector4(const std::string &name) const {
    return Get<DirectX::XMFLOAT4>(name);
  }

  ID3D11ShaderResourceView *GetTexture(const std::string &name) const {
    return Get<ID3D11ShaderResourceView *>(name);
  }

  bool HasParameter(const std::string &name) const {
    return parameters_.find(name) != parameters_.end();
  }

private:
  std::unordered_map<std::string, std::any> parameters_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\ShaderParameterLayout.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum class ShaderParameterType { Matrix, Vector3, Vector4, Texture, Float };

class ShaderParameterInfo {
public:
  ShaderParameterInfo(std::string name, ShaderParameterType type) {
    this->name = name;
    this->type = type;
  }

public:
  std::string name;
  ShaderParameterType type;
};

class ShaderParameterLayout {
public:
  void AddParameter(const std::string &name, ShaderParameterType type) {
    parameters_.push_back(ShaderParameterInfo(name, type));
  }

  const std::vector<ShaderParameterInfo> &GetParameters() const {
    return parameters_;
  }

private:
  std::vector<ShaderParameterInfo> parameters_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\shadowshaderclass.h

#ifndef _SHADOWSHADERCLASS_H_
#define _SHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <fstream>

#include "IShader.h"

class ShadowShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX lightView;
    DirectX::XMMATRIX lightProjection;
  };

  struct LightBufferType2 {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  ShadowShaderClass() = default;

  ShadowShaderClass(const ShadowShaderClass &) = delete;

  ShadowShaderClass &operator=(const ShadowShaderClass &) = delete;

  ~ShadowShaderClass() = default;

public:
  bool Initialize(HWND hwnd) override;

  void Shutdown() override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;

private:
  bool InitializeShader(HWND hwnd, const std::wstring &vsFilename,
                        const std::wstring &psFilename);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd,
                                const std::wstring &shaderFilename);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           const DirectX::XMMATRIX &lightViewMatrix,
                           const DirectX::XMMATRIX &lightProjectionMatrix,
                           ID3D11ShaderResourceView *depthMapTexture,
                           const DirectX::XMFLOAT3 &lightPosition) const;

  void RenderShader(int indexCount) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffee_2;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\softshadowshaderclass.h

#ifndef _SOFTSHADOWSHADERCLASS_H_
#define _SOFTSHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class SoftShadowShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct LightBufferType {
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
  };

  struct LightBufferType2 {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  SoftShadowShaderClass() = default;

  SoftShadowShaderClass(const SoftShadowShaderClass &) = delete;

  ~SoftShadowShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int indexCount,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *,
                           const DirectX::XMFLOAT3 &, const DirectX::XMFLOAT4 &,
                           const DirectX::XMFLOAT4 &) const;
  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleStateWrap;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleStateClamp;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer2;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\StandardRenderGroup.h
#pragma once

#include "GameObject.h"
#include "IRenderGroup.h"

class StandardRenderGroup : public IRenderGroup {
public:
  void PreRender() override {}

  void PostRender() override {}

  const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const override {
    return renderables_;
  }

  void AddRenderable(std::shared_ptr<IRenderable> renderable);

  void AddGameObject(std::shared_ptr<GameObject> gameObject);

private:
  std::vector<std::shared_ptr<IRenderable>> renderables_;
  std::vector<std::shared_ptr<GameObject>> gameObjects_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\System.h
#pragma once

#include "../CommonFramework/SystemBase.h"

class GraphicsClass;
class PositionClass;

class System : public SystemBase {
public:
  System();

  System(const System &rhs) = delete;

  System &operator=(const System &rhs) = delete;

  virtual ~System();

public:
  virtual bool Initialize() override;

  virtual void Shutdown() override;

  virtual bool Frame() override;

private:
  bool HandleInput(float frame_time);

private:
  GraphicsClass *graphics_;

  PositionClass *position_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System *ApplicationInstance = nullptr;

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\textureclass.h
#pragma once
//
// #include <DDSTextureLoader.h>
// #include <d3d11.h>
// using namespace DirectX;
//
// class TextureClass {
// public:
//  TextureClass();
//  TextureClass(const TextureClass &);
//  ~TextureClass();
//
//  bool Initialize(WCHAR *);
//  void Shutdown();
//
//  ID3D11ShaderResourceView *GetTexture();
//
// private:
//  ID3D11ShaderResourceView *texture_;
//};

#include <d3d11.h>
#include <wrl/client.h>

class TextureClass {
public:
  explicit TextureClass() = default;

  TextureClass(const TextureClass &) = delete;

  TextureClass &operator=(const TextureClass &) = delete;

  TextureClass(TextureClass &&) noexcept = default;

  TextureClass &operator=(TextureClass &&) noexcept = default;

  virtual ~TextureClass() = default;

public:
  bool Initialize(const WCHAR *filename);

  void Shutdown();

  ID3D11ShaderResourceView *GetTexture() const;

private:
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\textureshaderclass.h
#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class TextureShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  TextureShaderClass() = default;

  TextureShaderClass(const TextureShaderClass &) = delete;

  ~TextureShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int indexCount,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\verticalblurshaderclass.h

#ifndef _VERTICALBLURSHADERCLASS_H_
#define _VERTICALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class VerticalBlurShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenHeight;
    DirectX::XMFLOAT3 padding;
  };

public:
  VerticalBlurShaderClass() = default;

  VerticalBlurShaderClass(const VerticalBlurShaderClass &) = delete;

  ~VerticalBlurShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *, float) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> screen_size_buffer_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\depthshaderclass.cpp
#include "../CommonFramework/DirectX11Device.h"
#include "depthshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool DepthShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"./depth.vs", L"./depth.ps");
  if (!result) {
    return false;
  }

  return true;
}

void DepthShaderClass::Shutdown() { ShutdownShader(); }

bool DepthShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool DepthShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                        WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
  unsigned int numElements;
  D3D11_BUFFER_DESC matrixBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "DepthVertexShader",
                              "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(psFilename, NULL, NULL, "DepthPixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void DepthShaderClass::ShutdownShader() {}

void DepthShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                                HWND hwnd,
                                                WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool DepthShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  return true;
}

void DepthShaderClass::RenderShader(int indexCount) const {
  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\Graphics.cpp
#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "depthshaderclass.h"
#include "horizontalblurshaderclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "orthowindowclass.h"
#include "rendertextureclass.h"
#include "shadowshaderclass.h"
#include "softshadowshaderclass.h"
#include "textureshaderclass.h"
#include "verticalblurshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;
  int downSampleWidth, downSampleHeight;

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  {
    camera_ = (Camera *)_aligned_malloc(sizeof(Camera), 16);
    new (camera_) Camera();
    if (!camera_) {
      return false;
    }
    camera_->SetPosition(0.0f, -1.0f, -10.0f);
    camera_->Render();
    camera_->RenderBaseViewMatrix();
  }

  {
    // Create the cube model object.
    m_CubeModel = new ModelClass();
    if (!m_CubeModel) {
      return false;
    }

    // Initialize the cube model object.
    result = m_CubeModel->Initialize("./data/cube.txt", L"./data/wall01.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error",
                 MB_OK);
      return false;
    }
    // Set the position for the cube model.
    // m_CubeModel->SetPosition(-2.0f, 2.0f, 0.0f);
    m_CubeModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(-2.0f, 2.0f, 0.0f)));
  }

  {
    // Create the sphere model object.
    m_SphereModel = new ModelClass();
    if (!m_SphereModel) {
      return false;
    }

    // Initialize the sphere model object.
    result = m_SphereModel->Initialize("./data/sphere.txt", L"./data/ice.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the sphere model object.",
                 L"Error", MB_OK);
      return false;
    }

    // Set the position for the sphere model.
    // m_SphereModel->SetPosition(2.0f, 2.0f, 0.0f);
    m_SphereModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(2.0f, 2.0f, 0.0f)));
  }

  {
    // Create the ground model object.
    m_GroundModel = new ModelClass();
    if (!m_GroundModel) {
      return false;
    }

    // Initialize the ground model object.
    result =
        m_GroundModel->Initialize("./data/plane01.txt", L"./data/metal001.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the ground model object.",
                 L"Error", MB_OK);
      return false;
    }

    // Set the position for the ground model.
    // m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
    m_GroundModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(0.0f, 1.0f, 0.0f)));
  }

  {
    // Create the light object.
    light_ = (LightClass *)_aligned_malloc(sizeof(LightClass), 16);
    new (light_) LightClass();
    if (!light_) {
      return false;
    }

    // Initialize the light object.
    light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetLookAt(0.0f, 0.0f, 0.0f);
    light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
  }

  {
    // Create the render to texture object.
    render_texture_ =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (render_texture_) RenderTextureClass();
    if (!render_texture_) {
      return false;
    }

    // Initialize the render to texture object.
    result = render_texture_->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                         SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the render to texture object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the depth shader object.
    depth_shader_ =
        (DepthShaderClass *)_aligned_malloc(sizeof(DepthShaderClass), 16);
    new (depth_shader_) DepthShaderClass();
    if (!depth_shader_) {
      return false;
    }

    // Initialize the depth shader object.
    result = depth_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the depth shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the black and white render to texture object.
    m_BlackWhiteRenderTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_BlackWhiteRenderTexture) RenderTextureClass();
    if (!m_BlackWhiteRenderTexture) {
      return false;
    }

    // Initialize the black and white render to texture object.
    result = m_BlackWhiteRenderTexture->Initialize(
        SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the black and white render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the shadow shader object.
    m_ShadowShader =
        (ShadowShaderClass *)_aligned_malloc(sizeof(ShadowShaderClass), 16);
    new (m_ShadowShader) ShadowShaderClass();
    if (!m_ShadowShader) {
      return false;
    }

    // Initialize the shadow shader object.
    result = m_ShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  // Set the size to sample down to.
  downSampleWidth = SHADOWMAP_WIDTH / 2;
  downSampleHeight = SHADOWMAP_HEIGHT / 2;

  {
    // Create the down sample render to texture object.
    m_DownSampleTexure =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_DownSampleTexure) RenderTextureClass();
    if (!m_DownSampleTexure) {
      return false;
    }

    // Initialize the down sample render to texture object.
    result = m_DownSampleTexure->Initialize(downSampleWidth, downSampleHeight,
                                            100.0f, 1.0f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the down sample render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the small ortho window object.
    m_SmallWindow = new OrthoWindowClass();
    if (!m_SmallWindow) {
      return false;
    }

    // Initialize the small ortho window object.
    result = m_SmallWindow->Initialize(downSampleWidth, downSampleHeight);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the small ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the texture shader object.
    texture_shader_ =
        (TextureShaderClass *)_aligned_malloc(sizeof(TextureShaderClass), 16);
    new (texture_shader_) TextureShaderClass();
    if (!texture_shader_) {
      return false;
    }

    // Initialize the texture shader object.
    result = texture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the texture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the horizontal blur render to texture object.
    m_HorizontalBlurTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_HorizontalBlurTexture) RenderTextureClass();
    if (!m_HorizontalBlurTexture) {
      return false;
    }

    // Initialize the horizontal blur render to texture object.
    result = m_HorizontalBlurTexture->Initialize(
        downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the horizontal blur render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the horizontal blur shader object.
    m_HorizontalBlurShader = (HorizontalBlurShaderClass *)_aligned_malloc(
        sizeof(HorizontalBlurShaderClass), 16);
    new (m_HorizontalBlurShader) HorizontalBlurShaderClass();
    if (!m_HorizontalBlurShader) {
      return false;
    }

    // Initialize the horizontal blur shader object.
    result = m_HorizontalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the horizontal blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the vertical blur render to texture object.
    m_VerticalBlurTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_VerticalBlurTexture) RenderTextureClass();
    if (!m_VerticalBlurTexture) {
      return false;
    }

    // Initialize the vertical blur render to texture object.
    result = m_VerticalBlurTexture->Initialize(
        downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the vertical blur render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the vertical blur shader object.
    m_VerticalBlurShader = (VerticalBlurShaderClass *)_aligned_malloc(
        sizeof(VerticalBlurShaderClass), 16);
    new (m_VerticalBlurShader) VerticalBlurShaderClass();
    if (!m_VerticalBlurShader) {
      return false;
    }

    // Initialize the vertical blur shader object.
    result = m_VerticalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the vertical blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the up sample render to texture object.
    m_UpSampleTexure =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_UpSampleTexure) RenderTextureClass();
    if (!m_UpSampleTexure) {
      return false;
    }

    // Initialize the up sample render to texture object.
    result = m_UpSampleTexure->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                          SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd, L"Could not initialize the up sample render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the full screen ortho window object.
    m_FullScreenWindow = new OrthoWindowClass();
    if (!m_FullScreenWindow) {
      return false;
    }

    // Initialize the full screen ortho window object.
    result = m_FullScreenWindow->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the full screen ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the soft shadow shader object.
    m_SoftShadowShader = (SoftShadowShaderClass *)_aligned_malloc(
        sizeof(SoftShadowShaderClass), 16);
    new (m_SoftShadowShader) SoftShadowShaderClass();
    if (!m_SoftShadowShader) {
      return false;
    }

    // Initialize the soft shadow shader object.
    result = m_SoftShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the soft shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  // Release the soft shadow shader object.
  if (m_SoftShadowShader) {
    m_SoftShadowShader->Shutdown();
    m_SoftShadowShader->~SoftShadowShaderClass();
    _aligned_free(m_SoftShadowShader);
    m_SoftShadowShader = 0;
  }

  if (m_FullScreenWindow) {
    m_FullScreenWindow->Shutdown();
    delete m_FullScreenWindow;
    m_FullScreenWindow = 0;
  }

  if (m_UpSampleTexure) {
    m_UpSampleTexure->Shutdown();
    m_UpSampleTexure->~RenderTextureClass();
    _aligned_free(m_UpSampleTexure);
    m_UpSampleTexure = 0;
  }

  if (m_VerticalBlurShader) {
    m_VerticalBlurShader->Shutdown();
    m_VerticalBlurShader->~VerticalBlurShaderClass();
    _aligned_free(m_VerticalBlurShader);
    m_VerticalBlurShader = 0;
  }

  if (m_VerticalBlurTexture) {
    m_VerticalBlurTexture->Shutdown();
    m_VerticalBlurTexture->~RenderTextureClass();
    _aligned_free(m_VerticalBlurTexture);
    m_VerticalBlurTexture = 0;
  }

  if (m_HorizontalBlurShader) {
    m_HorizontalBlurShader->Shutdown();
    m_HorizontalBlurShader->~HorizontalBlurShaderClass();
    _aligned_free(m_HorizontalBlurShader);
    m_HorizontalBlurShader = 0;
  }

  if (m_HorizontalBlurTexture) {
    m_HorizontalBlurTexture->Shutdown();
    m_HorizontalBlurTexture->~RenderTextureClass();
    _aligned_free(m_HorizontalBlurTexture);
    m_HorizontalBlurTexture = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (m_SmallWindow) {
    m_SmallWindow->Shutdown();
    delete m_SmallWindow;
    m_SmallWindow = 0;
  }

  if (m_DownSampleTexure) {
    m_DownSampleTexure->Shutdown();
    m_DownSampleTexure->~RenderTextureClass();
    _aligned_free(m_DownSampleTexure);
    m_DownSampleTexure = 0;
  }

  // Release the shadow shader object.
  if (m_ShadowShader) {
    m_ShadowShader->Shutdown();
    m_ShadowShader->~ShadowShaderClass();
    _aligned_free(m_ShadowShader);
    m_ShadowShader = 0;
  }

  // Release the black and white render to texture.
  if (m_BlackWhiteRenderTexture) {
    m_BlackWhiteRenderTexture->Shutdown();
    m_BlackWhiteRenderTexture->~RenderTextureClass();
    _aligned_free(m_BlackWhiteRenderTexture);
    m_BlackWhiteRenderTexture = 0;
  }

  // Release the depth shader object.
  if (depth_shader_) {
    depth_shader_->Shutdown();
    depth_shader_->~DepthShaderClass();
    _aligned_free(depth_shader_);
    depth_shader_ = 0;
  }

  if (render_texture_) {
    render_texture_->Shutdown();
    render_texture_->~RenderTextureClass();
    _aligned_free(render_texture_);
    render_texture_ = 0;
  }

  if (light_) {
    light_->~LightClass();
    _aligned_free(light_);
    light_ = nullptr;
    ;
  }

  // Release the ground model object.
  if (m_GroundModel) {
    m_GroundModel->Shutdown();
    delete m_GroundModel;
    m_GroundModel = 0;
  }

  // Release the sphere model object.
  if (m_SphereModel) {
    m_SphereModel->Shutdown();
    delete m_SphereModel;
    m_SphereModel = 0;
  }

  // Release the cube model object.
  if (m_CubeModel) {
    m_CubeModel->Shutdown();
    delete m_CubeModel;
    m_CubeModel = 0;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

void GraphicsClass::Frame(float deltatime) {

  static float lightPositionX = -5.0f;

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);

  // Update the position of the light each frame.
  lightPositionX += 0.05f;
  if (lightPositionX > 5.0f) {
    lightPositionX = -5.0f;
  }

  // Update the position of the light.
  light_->SetPosition(lightPositionX, 8.0f, -5.0f);

  // Render the graphics scene.
  Render();
}

bool GraphicsClass::RenderSceneToTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  render_texture_->SetRenderTarget(directx_device_->GetDeviceContext());

  render_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  light_->GenerateViewMatrix();

  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", lightViewMatrix);
  parameters.SetMatrix("projectionMatrix", lightProjectionMatrix);

  m_CubeModel->Render(*depth_shader_, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_SphereModel->Render(*depth_shader_, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_GroundModel->Render(*depth_shader_, parameters);

  directx_device_->SetBackBufferRenderTarget();

  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderBlackAndWhiteShadows() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_BlackWhiteRenderTexture->SetRenderTarget(
      directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_BlackWhiteRenderTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Generate the light view matrix based on the light's position.
  light_->GenerateViewMatrix();

  // Get the world, view, and projection matrices from the camera and d3d
  // objects.
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  // Get the light's view and projection matrices from the light object.
  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  // Setup the world matrix for the cube model.
  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", viewMatrix);
  parameters.SetMatrix("projectionMatrix", projectionMatrix);
  parameters.SetMatrix("lightViewMatrix", lightViewMatrix);
  parameters.SetMatrix("lightProjectionMatrix", lightProjectionMatrix);
  parameters.SetTexture("depthMapTexture",
                        render_texture_->GetShaderResourceView());
  parameters.SetVector3("lightPosition", light_->GetPosition());
  // Render the cube model using the shadow shader.
  m_CubeModel->Render(*m_ShadowShader, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_SphereModel->Render(*m_ShadowShader, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_GroundModel->Render(*m_ShadowShader, parameters);

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::DownSampleTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  m_DownSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_DownSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions being that it is smaller.
  m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  // Render the small ortho window using the texture shader and the render to
  // texture of the scene as the texture resource.
  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("baseViewMatrix", baseViewMatrix);
  parameters.SetMatrix("orthoMatrix", orthoMatrix);
  parameters.SetTexture("texture",
                        m_BlackWhiteRenderTexture->GetShaderResourceView());

  auto result =
      texture_shader_->Render(m_SmallWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderHorizontalBlurToTexture() {

  // Store the screen width in a float that will be used in the horizontal blur
  // shader.
  auto screenSizeX = (float)(SHADOWMAP_WIDTH / 2);

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_HorizontalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_HorizontalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  // Render the small ortho window using the horizontal blur shader and the down
  // sampled render to texture resource.

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", baseViewMatrix);
  parameters.SetMatrix("projectionMatrix", orthoMatrix);
  parameters.SetFloat("screenWidth", screenSizeX);
  parameters.SetTexture("texture", m_DownSampleTexure->GetShaderResourceView());

  auto result = m_HorizontalBlurShader->Render(m_SmallWindow->GetIndexCount(),
                                               parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderVerticalBlurToTexture() {

  // Store the screen height in a float that will be used in the vertical blur
  // shader.
  auto screenSizeY = (float)(SHADOWMAP_HEIGHT / 2);

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_VerticalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_VerticalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", baseViewMatrix);
  parameters.SetMatrix("projectionMatrix", orthoMatrix);
  parameters.SetFloat("screenHeight", screenSizeY);
  parameters.SetTexture("texture",
                        m_HorizontalBlurTexture->GetShaderResourceView());

  // Render the small ortho window using the vertical blur shader and the
  // horizontal blurred render to texture resource.
  auto result =
      m_VerticalBlurShader->Render(m_SmallWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::UpSampleTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_UpSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_UpSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the full screen ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

  // Render the full screen ortho window using the texture shader and the small
  // sized final blurred render to texture resource.

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("baseViewMatrix", baseViewMatrix);
  parameters.SetMatrix("orthoMatrix", orthoMatrix);
  parameters.SetTexture("texture",
                        m_VerticalBlurTexture->GetShaderResourceView());

  auto result =
      texture_shader_->Render(m_FullScreenWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

void GraphicsClass::Render() {

  // First render the scene to a texture.
  auto result = RenderSceneToTexture();
  if (!result) {
    return;
  }

  // Next render the shadowed scene in black and white.
  result = RenderBlackAndWhiteShadows();
  if (!result) {
    return;
  }

  // Then down sample the black and white scene texture.
  result = DownSampleTexture();
  if (!result) {
    return;
  }

  // Perform a horizontal blur on the down sampled texture.
  result = RenderHorizontalBlurToTexture();
  if (!result) {
    return;
  }

  // Now perform a vertical blur on the texture.
  result = RenderVerticalBlurToTexture();
  if (!result) {
    return;
  }

  // Finally up sample the final blurred render to texture that can now be used
  // in the soft shadow shader.
  result = UpSampleTexture();
  if (!result) {
    return;
  }

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Clear the buffers to begin the scene.
  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world, view, and projection matrices from the camera and d3d
  // objects.
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  // Setup the world matrix for the cube model.
  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", viewMatrix);
  parameters.SetMatrix("projectionMatrix", projectionMatrix);
  parameters.SetTexture("shadowTexture",
                        m_UpSampleTexure->GetShaderResourceView());
  parameters.SetVector3("lightPosition", light_->GetPosition());
  parameters.SetTexture("texture", m_CubeModel->GetTexture());
  parameters.SetVector4("diffuseColor", light_->GetDiffuseColor());
  parameters.SetVector4("ambientColor", light_->GetAmbientColor());

  // Render the cube model using the soft shadow shader.
  m_CubeModel->Render(*m_SoftShadowShader, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();

  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetTexture("texture", m_SphereModel->GetTexture());

  m_SphereModel->Render(*m_SoftShadowShader, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();

  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetTexture("texture", m_GroundModel->GetTexture());

  m_GroundModel->Render(*m_SoftShadowShader, parameters);

  result =
      m_SoftShadowShader->Render(m_GroundModel->GetIndexCount(), parameters);
  if (!result) {
    return;
  }

  // Present the rendered scene to the screen.
  directx_device_->EndScene();
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\horizontalblurshaderclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "horizontalblurshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool HorizontalBlurShaderClass::Initialize(HWND hwnd) {
  bool result;

  result =
      InitializeShader(hwnd, L"./horizontalblur.vs", L"./horizontalblur.ps");
  if (!result) {
    return false;
  }

  return true;
}

void HorizontalBlurShaderClass::Shutdown() { ShutdownShader(); }

bool HorizontalBlurShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto screenWidth = parameters.GetFloat("screenWidth");
  auto texture = parameters.GetTexture("texture");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    texture, screenWidth);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool HorizontalBlurShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                                 WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
  unsigned int numElements;
  D3D11_SAMPLER_DESC samplerDesc;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_BUFFER_DESC screenSizeBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(
      vsFilename, NULL, NULL, "HorizontalBlurVertexShader", "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(
      psFilename, NULL, NULL, "HorizontalBlurPixelShader", "ps_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  polygonLayout[1].SemanticName = "TEXCOORD";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  polygonLayout[1].InputSlot = 0;
  polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[1].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
  screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  screenSizeBufferDesc.MiscFlags = 0;
  screenSizeBufferDesc.StructureByteStride = 0;

  result =
      device->CreateBuffer(&screenSizeBufferDesc, NULL, &screen_size_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void HorizontalBlurShaderClass::ShutdownShader() {}

void HorizontalBlurShaderClass::OutputShaderErrorMessage(
    ID3D10Blob *errorMessage, HWND hwnd, WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool HorizontalBlurShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    float screenWidth) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  MatrixBufferType *dataPtr;
  unsigned int buffer_number;
  ScreenSizeBufferType *dataPtr2;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  result = device_context->Map(screen_size_buffer_.Get(), 0,
                               D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr2 = (ScreenSizeBufferType *)mappedResource.pData;

  dataPtr2->screenWidth = screenWidth;
  dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

  device_context->Unmap(screen_size_buffer_.Get(), 0);

  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       screen_size_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &texture);

  return true;
}

void HorizontalBlurShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\lightclass.cpp

#include "lightclass.h"

LightClass::LightClass() {}

LightClass::LightClass(const LightClass &other) {}

LightClass::~LightClass() {}

void LightClass::SetAmbientColor(float red, float green, float blue,
                                 float alpha) {
  ambient_color_ = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetDiffuseColor(float red, float green, float blue,
                                 float alpha) {
  diffuse_color_ = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetPosition(float x, float y, float z) {
  light_position_ = XMFLOAT3(x, y, z);
}

void LightClass::SetLookAt(float x, float y, float z) {
  light_look_at_.x = x;
  light_look_at_.y = y;
  light_look_at_.z = z;
}

XMFLOAT4 LightClass::GetAmbientColor() { return ambient_color_; }

XMFLOAT4 LightClass::GetDiffuseColor() { return diffuse_color_; }

XMFLOAT3 LightClass::GetPosition() { return light_position_; }

void LightClass::GenerateViewMatrix() {
  XMVECTOR up;
  XMVECTOR position;
  XMVECTOR lookAt;

  // Setup the vector that points upwards.
  up.m128_f32[0] = 0.0f;
  up.m128_f32[1] = 1.0f;
  up.m128_f32[2] = 0.0f;
  up.m128_f32[3] = 1.0f;

  position.m128_f32[0] = light_position_.x;
  position.m128_f32[1] = light_position_.y;
  position.m128_f32[2] = light_position_.z;
  position.m128_f32[3] = 1.0f;

  lookAt.m128_f32[0] = light_look_at_.x;
  lookAt.m128_f32[1] = light_look_at_.y;
  lookAt.m128_f32[2] = light_look_at_.z;
  lookAt.m128_f32[3] = 1.0f;

  // Create the view matrix from the three vectors.
  light_viewMatrix_ = XMMatrixLookAtLH(position, lookAt, up);
}

void LightClass::GenerateProjectionMatrix(float screenDepth, float screenNear) {
  float fieldOfView, screenAspect;

  // Setup field of view and screen aspect for a square light source.
  fieldOfView = (float)XM_PI / 2.0f;
  screenAspect = 1.0f;

  // Create the projection matrix for the light.
  projection_matrix_ = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect,
                                                screenNear, screenDepth);
}

void LightClass::GetViewMatrix(XMMATRIX &viewMatrix) {
  viewMatrix = light_viewMatrix_;
}

void LightClass::GetProjectionMatrix(XMMATRIX &projectionMatrix) {
  projectionMatrix = projection_matrix_;
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\main.cpp
#include "System.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline,
                   int iCmdshow) {

  System *system = new System();
  if (!system) {
    return 0;
  }

  bool result = system->Initialize();
  if (result) {
    system->Run();
  }

  system->Shutdown();
  delete system;
  system = nullptr;

  return 0;
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\merged_cpp_files.cpp

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\depthshaderclass.h
#ifndef _DEPTHSHADERCLASS_H_
#define _DEPTHSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class DepthShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  DepthShaderClass() = default;

  DepthShaderClass(const DepthShaderClass &) = delete;

  ~DepthShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\GameObject.h
#pragma once

#include "modelclass.h"
#include <DirectXMath.h>
#include <memory>

class GameObject {
public:
  GameObject(std::shared_ptr<ModelClass> model);

public:
  void SetPosition(const DirectX::XMFLOAT3 &position);

  void SetRotation(const DirectX::XMFLOAT3 &rotation);

  void SetScale(const DirectX::XMFLOAT3 &scale);

  void Update(float deltaTime);

  DirectX::XMMATRIX GetWorldMatrix() const;

  std::shared_ptr<ModelClass> GetModel() const;

private:
  std::shared_ptr<ModelClass> model_;
  DirectX::XMFLOAT3 position_;
  DirectX::XMFLOAT3 rotation_;
  DirectX::XMFLOAT3 scale_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\Graphics.h
#pragma once

#include "../CommonFramework/GraphicsBase.h"

const int SHADOWMAP_WIDTH = 1024;
const int SHADOWMAP_HEIGHT = 1024;

class DirectX11Device;
class Camera;
class ModelClass;
class DepthShaderClass;
class ShadowShaderClass;
class LightClass;
class RenderTextureClass;
class OrthoWindowClass;
class HorizontalBlurShaderClass;
class VerticalBlurShaderClass;
class SoftShadowShaderClass;
class TextureShaderClass;

class GraphicsClass : public GraphicsBase {
public:
  GraphicsClass();

  GraphicsClass(const GraphicsClass &rhs) = delete;

  GraphicsClass &operator=(const GraphicsClass &rhs) = delete;

  virtual ~GraphicsClass();

public:
  virtual bool Initialize(int, int, HWND) override;

  virtual void Shutdown() override;

  virtual void Frame(float deltatime) override;

  virtual void Render() override;

public:
  void SetPosition(float x, float y, float z) {
    pos_x_ = x;
    pos_y_ = y;
    pos_z_ = z;
  }

  void SetRotation(float x, float y, float z) {
    rot_x_ = x;
    rot_y_ = y;
    rot_z_ = z;
  }

private:
  bool RenderSceneToTexture();

  bool RenderBlackAndWhiteShadows();

  bool DownSampleTexture();

  bool RenderHorizontalBlurToTexture();

  bool RenderVerticalBlurToTexture();

  bool UpSampleTexture();

private:
  float pos_x_ = 0.0f, pos_y_ = 0.0f, pos_z_ = 0.0f;

  float rot_x_ = 0.0f, rot_y_ = 0.0f, rot_z_ = 0.0f;

  Camera *camera_ = nullptr;

  ModelClass *m_CubeModel, *m_GroundModel, *m_SphereModel;

  LightClass *light_;

  RenderTextureClass *render_texture_, *m_BlackWhiteRenderTexture,
      *m_DownSampleTexure;

  RenderTextureClass *m_HorizontalBlurTexture, *m_VerticalBlurTexture,
      *m_UpSampleTexure;

  DepthShaderClass *depth_shader_;

  ShadowShaderClass *m_ShadowShader;

  OrthoWindowClass *m_SmallWindow, *m_FullScreenWindow;

  TextureShaderClass *texture_shader_;

  HorizontalBlurShaderClass *m_HorizontalBlurShader;

  VerticalBlurShaderClass *m_VerticalBlurShader;

  SoftShadowShaderClass *m_SoftShadowShader;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\horizontalblurshaderclass.h

#ifndef _HORIZONTALBLURSHADERCLASS_H_
#define _HORIZONTALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class HorizontalBlurShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenWidth;
    DirectX::XMFLOAT3 padding;
  };

public:
  HorizontalBlurShaderClass() = default;

  HorizontalBlurShaderClass(const HorizontalBlurShaderClass &) = delete;

  ~HorizontalBlurShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *, float) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> screen_size_buffer_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\IRenderable.h
#pragma once

#include <DirectXMath.h>

#include "ShaderParameterContainer.h"

class IShader;

class IRenderable {
public:
  virtual ~IRenderable() = default;

  virtual void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const = 0;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\IRenderGroup.h
#pragma once

#include "IRenderable.h"
#include <memory>
#include <vector>

class IRenderGroup {
public:
  virtual ~IRenderGroup() = default;

public:
  virtual void PreRender() = 0;

  virtual void PostRender() = 0;

  virtual const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const = 0;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\IShader.h
#pragma once

#include <Windows.h>

#include "ShaderParameterContainer.h"

class IShader {
public:
  virtual ~IShader() = default;

  virtual bool Initialize(HWND hwnd) = 0;

  virtual void Shutdown() = 0;

  virtual bool Render(int indexCount,
                      const ShaderParameterContainer &parameters) const = 0;

  // virtual const ShaderParameterLayout& GetParameterLayout() const = 0;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\lightclass.h
#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class LightClass {
public:
  LightClass();
  LightClass(const LightClass &);
  ~LightClass();

  void SetAmbientColor(float, float, float, float);
  void SetDiffuseColor(float, float, float, float);
  void SetPosition(float, float, float);
  void SetLookAt(float, float, float);

  XMFLOAT4 GetAmbientColor();
  XMFLOAT4 GetDiffuseColor();
  XMFLOAT3 GetPosition();

  void GenerateViewMatrix();
  void GenerateProjectionMatrix(float, float);

  void GetViewMatrix(XMMATRIX &);
  void GetProjectionMatrix(XMMATRIX &);

private:
  XMFLOAT4 ambient_color_;
  XMFLOAT4 diffuse_color_;
  XMFLOAT3 light_position_;
  XMFLOAT3 light_look_at_;
  XMMATRIX light_viewMatrix_;
  XMMATRIX projection_matrix_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\modelclass.h

#pragma once

#include "IRenderable.h"
#include "ShaderParameterLayout.h"
#include "textureclass.h"

#include <d3d11.h>
#include <wrl/client.h>

#include <memory>
#include <string>
#include <vector>

class ModelClass : public IRenderable {
public:
  ModelClass() = default;

  ModelClass(const ModelClass &) = delete;

  ModelClass &operator=(const ModelClass &) = delete;

  ~ModelClass();

public:
  bool Initialize(const std::string &modelFilename,
                  const std::wstring &textureFilename);

  void Shutdown();

  void
  Render(const IShader &shader,
         const ShaderParameterContainer &parameterContainer) const override;

  int GetIndexCount() const { return index_count_; }

  ID3D11ShaderResourceView *GetTexture() const;

  DirectX::XMMATRIX GetWorldMatrix() const { return world_matrix_; }

  void SetWorldMatrix(const DirectX::XMMATRIX &worldMatrix) {
    world_matrix_ = worldMatrix;
  }

private:
  bool InitializeBuffers();

  void ShutdownBuffers();

  void RenderBuffers() const;

  bool LoadTexture(const std::wstring &filename);

  void ReleaseTexture();

  bool LoadModel(const std::string &filename);

  void ReleaseModel();

private:
  struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 texture;
    DirectX::XMFLOAT3 normal;
  };

  struct ModelType {
    float x, y, z;
    float tu, tv;
    float nx, ny, nz;
  };

  Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;

  int vertex_count_ = 0;
  int index_count_ = 0;

  std::unique_ptr<TextureClass> texture_;

  std::vector<ModelType> model_;

  DirectX::XMMATRIX world_matrix_ = DirectX::XMMatrixIdentity();
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\orthowindowclass.h

#ifndef _ORTHOWINDOWCLASS_H_
#define _ORTHOWINDOWCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

class OrthoWindowClass {
private:
  struct VertexType {
    XMFLOAT3 position;
    XMFLOAT2 texture;
  };

public:
  OrthoWindowClass();
  OrthoWindowClass(const OrthoWindowClass &);
  ~OrthoWindowClass();

  bool Initialize(int, int);
  void Shutdown();
  void Render(ID3D11DeviceContext *);

  int GetIndexCount();

private:
  bool InitializeBuffers(int, int);
  void ShutdownBuffers();
  void RenderBuffers(ID3D11DeviceContext *);

private:
  ID3D11Buffer *vertex_buffer_, *index_buffer_;
  int vertex_count_, index_count_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\positionclass.h
#pragma once

#include <math.h>

class PositionClass {
public:
  PositionClass();
  PositionClass(const PositionClass &);
  ~PositionClass();

  void SetPosition(float, float, float);
  void SetRotation(float, float, float);

  void GetPosition(float &, float &, float &);
  void GetRotation(float &, float &, float &);

  void SetFrameTime(float);

  void MoveForward(bool);
  void MoveBackward(bool);
  void MoveUpward(bool);
  void MoveDownward(bool);
  void TurnLeft(bool);
  void TurnRight(bool);
  void LookUpward(bool);
  void LookDownward(bool);

private:
  float position_x_, position_y_, position_z_;
  float rotation_x_, rotation_y_, rotation_z_;

  float frame_time_;

  float m_forwardSpeed, m_backwardSpeed;
  float m_upwardSpeed, m_downwardSpeed;
  float left_turning_speed_, right_turning_speed_;
  float m_lookUpSpeed, m_lookDownSpeed;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\RenderPipeline.h
#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "IShader.h"
#include "Scene.h"
#include "ShaderParameterLayout.h"

class RenderPipeline {
public:
  bool Initialize(HWND hwnd);

  void Shutdown();

  bool Render(const Scene &scene) const;

  void AddShader(const std::string &name, std::unique_ptr<IShader> shader);

  void RemoveShader(const std::string &name);

  IShader *GetShader(const std::string &name) const;

  void SetActiveShader(const std::string &name);

private:
  std::unordered_map<std::string, std::unique_ptr<IShader>> shaders_;
  std::unordered_map<std::string, ShaderParameterLayout> shader_layouts_;
  std::string active_shader_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\RenderSystem.h
#pragma once

#include <memory>

#include "RenderPipeline.h"

class RenderSystem {
public:
  bool Initialize(HWND hwnd);

  void Shutdown();

  void Render(const Scene &scene);

private:
  std::unique_ptr<RenderPipeline> pipeline_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\rendertextureclass.h
#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
using namespace DirectX;

class RenderTextureClass {
public:
  RenderTextureClass();
  RenderTextureClass(const RenderTextureClass &);
  ~RenderTextureClass();

  bool Initialize(int, int, float, float);
  void Shutdown();

  void SetRenderTarget(ID3D11DeviceContext *);
  void ClearRenderTarget(float, float, float, float);
  ID3D11ShaderResourceView *GetShaderResourceView();
  void GetProjectionMatrix(XMMATRIX &);
  void GetOrthoMatrix(XMMATRIX &);

private:
  ID3D11Texture2D *render_target_texture_;
  ID3D11RenderTargetView *render_target_view_;
  ID3D11ShaderResourceView *shader_resource_view_;
  ID3D11Texture2D *depth_stencil_buffer_;
  ID3D11DepthStencilView *depth_stencil_view_;
  D3D11_VIEWPORT viewport_;
  XMMATRIX projection_matrix_;
  XMMATRIX ortho_matrix_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\Scene.h Scene.h
#pragma once
#include <memory>
#include <vector>

#include "../CommonFramework/Camera.h"
#include "IRenderGroup.h"
#include "lightclass.h"

class Scene {
public:
  void AddRenderGroup(std::shared_ptr<IRenderGroup> renderGroup);

  void SetCamera(std::shared_ptr<Camera> camera);

  void AddLight(std::shared_ptr<LightClass> light);

  const std::vector<std::shared_ptr<IRenderGroup>> &GetRenderGroups() const {
    return renderGroups_;
  }

  std::shared_ptr<Camera> GetCamera() const { return camera_; }

  const std::vector<std::shared_ptr<LightClass>> &GetLights() const {
    return lights_;
  }

private:
  std::vector<std::shared_ptr<IRenderGroup>> renderGroups_;
  std::shared_ptr<Camera> camera_;
  std::vector<std::shared_ptr<LightClass>> lights_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\ShaderParameterContainer.h
#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

#include <any>
#include <string>
#include <unordered_map>

class ShaderParameterContainer {
public:
  void SetFloat(const std::string &name, float f);

  void SetMatrix(const std::string &name, const DirectX::XMMATRIX &matrix);

  void SetVector3(const std::string &name, const DirectX::XMFLOAT3 &vector);

  void SetVector4(const std::string &name, const DirectX::XMFLOAT4 &vector);

  void SetTexture(const std::string &name, ID3D11ShaderResourceView *texture);

  template <typename T> T Get(const std::string &name) const {
    auto it = parameters_.find(name);
    if (it == parameters_.end()) {
      throw std::runtime_error("Parameter not found: " + name);
    }
    try {
      return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast &) {
      throw std::runtime_error("Type mismatch for parameter: " + name);
    }
  }

  float GetFloat(const std::string &name) const { return Get<float>(name); }

  DirectX::XMMATRIX GetMatrix(const std::string &name) const {
    return Get<DirectX::XMMATRIX>(name);
  }

  DirectX::XMFLOAT3 GetVector3(const std::string &name) const {
    return Get<DirectX::XMFLOAT3>(name);
  }

  DirectX::XMFLOAT4 GetVector4(const std::string &name) const {
    return Get<DirectX::XMFLOAT4>(name);
  }

  ID3D11ShaderResourceView *GetTexture(const std::string &name) const {
    return Get<ID3D11ShaderResourceView *>(name);
  }

  bool HasParameter(const std::string &name) const {
    return parameters_.find(name) != parameters_.end();
  }

private:
  std::unordered_map<std::string, std::any> parameters_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\ShaderParameterLayout.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum class ShaderParameterType { Matrix, Vector3, Vector4, Texture, Float };

class ShaderParameterInfo {
public:
  ShaderParameterInfo(std::string name, ShaderParameterType type) {
    this->name = name;
    this->type = type;
  }

public:
  std::string name;
  ShaderParameterType type;
};

class ShaderParameterLayout {
public:
  void AddParameter(const std::string &name, ShaderParameterType type) {
    parameters_.push_back(ShaderParameterInfo(name, type));
  }

  const std::vector<ShaderParameterInfo> &GetParameters() const {
    return parameters_;
  }

private:
  std::vector<ShaderParameterInfo> parameters_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\shadowshaderclass.h

#ifndef _SHADOWSHADERCLASS_H_
#define _SHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <fstream>

#include "IShader.h"

class ShadowShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
    DirectX::XMMATRIX lightView;
    DirectX::XMMATRIX lightProjection;
  };

  struct LightBufferType2 {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  ShadowShaderClass() = default;

  ShadowShaderClass(const ShadowShaderClass &) = delete;

  ShadowShaderClass &operator=(const ShadowShaderClass &) = delete;

  ~ShadowShaderClass() = default;

public:
  bool Initialize(HWND hwnd) override;

  void Shutdown() override;

  bool Render(int indexCount,
              const ShaderParameterContainer &parameters) const override;

private:
  bool InitializeShader(HWND hwnd, const std::wstring &vsFilename,
                        const std::wstring &psFilename);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *errorMessage, HWND hwnd,
                                const std::wstring &shaderFilename);

  bool SetShaderParameters(const DirectX::XMMATRIX &worldMatrix,
                           const DirectX::XMMATRIX &viewMatrix,
                           const DirectX::XMMATRIX &projectionMatrix,
                           const DirectX::XMMATRIX &lightViewMatrix,
                           const DirectX::XMMATRIX &lightProjectionMatrix,
                           ID3D11ShaderResourceView *depthMapTexture,
                           const DirectX::XMFLOAT3 &lightPosition) const;

  void RenderShader(int indexCount) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffee_2;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\softshadowshaderclass.h

#ifndef _SOFTSHADOWSHADERCLASS_H_
#define _SOFTSHADOWSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class SoftShadowShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct LightBufferType {
    DirectX::XMFLOAT4 ambientColor;
    DirectX::XMFLOAT4 diffuseColor;
  };

  struct LightBufferType2 {
    DirectX::XMFLOAT3 lightPosition;
    float padding;
  };

public:
  SoftShadowShaderClass() = default;

  SoftShadowShaderClass(const SoftShadowShaderClass &) = delete;

  ~SoftShadowShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int indexCount,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *,
                           ID3D11ShaderResourceView *,
                           const DirectX::XMFLOAT3 &, const DirectX::XMFLOAT4 &,
                           const DirectX::XMFLOAT4 &) const;
  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleStateWrap;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleStateClamp;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> light_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer2;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\StandardRenderGroup.h
#pragma once

#include "GameObject.h"
#include "IRenderGroup.h"

class StandardRenderGroup : public IRenderGroup {
public:
  void PreRender() override {}

  void PostRender() override {}

  const std::vector<std::shared_ptr<IRenderable>> &
  GetRenderables() const override {
    return renderables_;
  }

  void AddRenderable(std::shared_ptr<IRenderable> renderable);

  void AddGameObject(std::shared_ptr<GameObject> gameObject);

private:
  std::vector<std::shared_ptr<IRenderable>> renderables_;
  std::vector<std::shared_ptr<GameObject>> gameObjects_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\System.h
#pragma once

#include "../CommonFramework/SystemBase.h"

class GraphicsClass;
class PositionClass;

class System : public SystemBase {
public:
  System();

  System(const System &rhs) = delete;

  System &operator=(const System &rhs) = delete;

  virtual ~System();

public:
  virtual bool Initialize() override;

  virtual void Shutdown() override;

  virtual bool Frame() override;

private:
  bool HandleInput(float frame_time);

private:
  GraphicsClass *graphics_;

  PositionClass *position_;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System *ApplicationInstance = nullptr;

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\textureclass.h
#pragma once
//
// #include <DDSTextureLoader.h>
// #include <d3d11.h>
// using namespace DirectX;
//
// class TextureClass {
// public:
//  TextureClass();
//  TextureClass(const TextureClass &);
//  ~TextureClass();
//
//  bool Initialize(WCHAR *);
//  void Shutdown();
//
//  ID3D11ShaderResourceView *GetTexture();
//
// private:
//  ID3D11ShaderResourceView *texture_;
//};

#include <d3d11.h>
#include <wrl/client.h>

class TextureClass {
public:
  explicit TextureClass() = default;

  TextureClass(const TextureClass &) = delete;

  TextureClass &operator=(const TextureClass &) = delete;

  TextureClass(TextureClass &&) noexcept = default;

  TextureClass &operator=(TextureClass &&) noexcept = default;

  virtual ~TextureClass() = default;

public:
  bool Initialize(const WCHAR *filename);

  void Shutdown();

  ID3D11ShaderResourceView *GetTexture() const;

private:
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\textureshaderclass.h
#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class TextureShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

public:
  TextureShaderClass() = default;

  TextureShaderClass(const TextureShaderClass &) = delete;

  ~TextureShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int indexCount,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;
};

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\verticalblurshaderclass.h

#ifndef _VERTICALBLURSHADERCLASS_H_
#define _VERTICALBLURSHADERCLASS_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "IShader.h"

class VerticalBlurShaderClass : public IShader {
private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
  };

  struct ScreenSizeBufferType {
    float screenHeight;
    DirectX::XMFLOAT3 padding;
  };

public:
  VerticalBlurShaderClass() = default;

  VerticalBlurShaderClass(const VerticalBlurShaderClass &) = delete;

  ~VerticalBlurShaderClass() = default;

public:
  bool Initialize(HWND) override;

  void Shutdown() override;

  bool
  Render(int,
         const ShaderParameterContainer &parameterContainer) const override;

private:
  bool InitializeShader(HWND, WCHAR *, WCHAR *);

  void ShutdownShader();

  void OutputShaderErrorMessage(ID3D10Blob *, HWND, WCHAR *);

  bool SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &,
                           const DirectX::XMMATRIX &,
                           ID3D11ShaderResourceView *, float) const;

  void RenderShader(int) const;

private:
  Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> sample_state_;

  Microsoft::WRL::ComPtr<ID3D11Buffer> matrix_buffer_;
  Microsoft::WRL::ComPtr<ID3D11Buffer> screen_size_buffer_;
};

#endif

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\depthshaderclass.cpp
#include "../CommonFramework/DirectX11Device.h"
#include "depthshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool DepthShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"./depth.vs", L"./depth.ps");
  if (!result) {
    return false;
  }

  return true;
}

void DepthShaderClass::Shutdown() { ShutdownShader(); }

bool DepthShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool DepthShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                        WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
  unsigned int numElements;
  D3D11_BUFFER_DESC matrixBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "DepthVertexShader",
                              "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(psFilename, NULL, NULL, "DepthPixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void DepthShaderClass::ShutdownShader() {}

void DepthShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                                HWND hwnd,
                                                WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool DepthShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  return true;
}

void DepthShaderClass::RenderShader(int indexCount) const {
  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\Graphics.cpp
#include "Graphics.h"

#include <new>

#include "../CommonFramework/Camera.h"
#include "../CommonFramework/DirectX11Device.h"
#include "../CommonFramework/Input.h"
#include "../CommonFramework/TypeDefine.h"

#include "depthshaderclass.h"
#include "horizontalblurshaderclass.h"
#include "lightclass.h"
#include "modelclass.h"
#include "orthowindowclass.h"
#include "rendertextureclass.h"
#include "shadowshaderclass.h"
#include "softshadowshaderclass.h"
#include "textureshaderclass.h"
#include "verticalblurshaderclass.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd) {

  bool result;
  int downSampleWidth, downSampleHeight;

  auto directx11_device_ = DirectX11Device::GetD3d11DeviceInstance();

  if (!(directx11_device_->Initialize(screenWidth, screenHeight, VSYNC_ENABLED,
                                      hwnd, FULL_SCREEN, SCREEN_DEPTH,
                                      SCREEN_NEAR))) {
    MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
    return false;
  }

  {
    camera_ = (Camera *)_aligned_malloc(sizeof(Camera), 16);
    new (camera_) Camera();
    if (!camera_) {
      return false;
    }
    camera_->SetPosition(0.0f, -1.0f, -10.0f);
    camera_->Render();
    camera_->RenderBaseViewMatrix();
  }

  {
    // Create the cube model object.
    m_CubeModel = new ModelClass();
    if (!m_CubeModel) {
      return false;
    }

    // Initialize the cube model object.
    result = m_CubeModel->Initialize("./data/cube.txt", L"./data/wall01.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the cube model object.", L"Error",
                 MB_OK);
      return false;
    }
    // Set the position for the cube model.
    // m_CubeModel->SetPosition(-2.0f, 2.0f, 0.0f);
    m_CubeModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(-2.0f, 2.0f, 0.0f)));
  }

  {
    // Create the sphere model object.
    m_SphereModel = new ModelClass();
    if (!m_SphereModel) {
      return false;
    }

    // Initialize the sphere model object.
    result = m_SphereModel->Initialize("./data/sphere.txt", L"./data/ice.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the sphere model object.",
                 L"Error", MB_OK);
      return false;
    }

    // Set the position for the sphere model.
    // m_SphereModel->SetPosition(2.0f, 2.0f, 0.0f);
    m_SphereModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(2.0f, 2.0f, 0.0f)));
  }

  {
    // Create the ground model object.
    m_GroundModel = new ModelClass();
    if (!m_GroundModel) {
      return false;
    }

    // Initialize the ground model object.
    result =
        m_GroundModel->Initialize("./data/plane01.txt", L"./data/metal001.dds");
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the ground model object.",
                 L"Error", MB_OK);
      return false;
    }

    // Set the position for the ground model.
    // m_GroundModel->SetPosition(0.0f, 1.0f, 0.0f);
    m_GroundModel->SetWorldMatrix(
        std::move(XMMatrixTranslation(0.0f, 1.0f, 0.0f)));
  }

  {
    // Create the light object.
    light_ = (LightClass *)_aligned_malloc(sizeof(LightClass), 16);
    new (light_) LightClass();
    if (!light_) {
      return false;
    }

    // Initialize the light object.
    light_->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
    light_->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    light_->SetLookAt(0.0f, 0.0f, 0.0f);
    light_->GenerateProjectionMatrix(SCREEN_DEPTH, SCREEN_NEAR);
  }

  {
    // Create the render to texture object.
    render_texture_ =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (render_texture_) RenderTextureClass();
    if (!render_texture_) {
      return false;
    }

    // Initialize the render to texture object.
    result = render_texture_->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                         SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the render to texture object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the depth shader object.
    depth_shader_ =
        (DepthShaderClass *)_aligned_malloc(sizeof(DepthShaderClass), 16);
    new (depth_shader_) DepthShaderClass();
    if (!depth_shader_) {
      return false;
    }

    // Initialize the depth shader object.
    result = depth_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the depth shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the black and white render to texture object.
    m_BlackWhiteRenderTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_BlackWhiteRenderTexture) RenderTextureClass();
    if (!m_BlackWhiteRenderTexture) {
      return false;
    }

    // Initialize the black and white render to texture object.
    result = m_BlackWhiteRenderTexture->Initialize(
        SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the black and white render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the shadow shader object.
    m_ShadowShader =
        (ShadowShaderClass *)_aligned_malloc(sizeof(ShadowShaderClass), 16);
    new (m_ShadowShader) ShadowShaderClass();
    if (!m_ShadowShader) {
      return false;
    }

    // Initialize the shadow shader object.
    result = m_ShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  // Set the size to sample down to.
  downSampleWidth = SHADOWMAP_WIDTH / 2;
  downSampleHeight = SHADOWMAP_HEIGHT / 2;

  {
    // Create the down sample render to texture object.
    m_DownSampleTexure =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_DownSampleTexure) RenderTextureClass();
    if (!m_DownSampleTexure) {
      return false;
    }

    // Initialize the down sample render to texture object.
    result = m_DownSampleTexure->Initialize(downSampleWidth, downSampleHeight,
                                            100.0f, 1.0f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the down sample render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the small ortho window object.
    m_SmallWindow = new OrthoWindowClass();
    if (!m_SmallWindow) {
      return false;
    }

    // Initialize the small ortho window object.
    result = m_SmallWindow->Initialize(downSampleWidth, downSampleHeight);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the small ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the texture shader object.
    texture_shader_ =
        (TextureShaderClass *)_aligned_malloc(sizeof(TextureShaderClass), 16);
    new (texture_shader_) TextureShaderClass();
    if (!texture_shader_) {
      return false;
    }

    // Initialize the texture shader object.
    result = texture_shader_->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the texture shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the horizontal blur render to texture object.
    m_HorizontalBlurTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_HorizontalBlurTexture) RenderTextureClass();
    if (!m_HorizontalBlurTexture) {
      return false;
    }

    // Initialize the horizontal blur render to texture object.
    result = m_HorizontalBlurTexture->Initialize(
        downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the horizontal blur render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the horizontal blur shader object.
    m_HorizontalBlurShader = (HorizontalBlurShaderClass *)_aligned_malloc(
        sizeof(HorizontalBlurShaderClass), 16);
    new (m_HorizontalBlurShader) HorizontalBlurShaderClass();
    if (!m_HorizontalBlurShader) {
      return false;
    }

    // Initialize the horizontal blur shader object.
    result = m_HorizontalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the horizontal blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the vertical blur render to texture object.
    m_VerticalBlurTexture =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_VerticalBlurTexture) RenderTextureClass();
    if (!m_VerticalBlurTexture) {
      return false;
    }

    // Initialize the vertical blur render to texture object.
    result = m_VerticalBlurTexture->Initialize(
        downSampleWidth, downSampleHeight, SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd,
          L"Could not initialize the vertical blur render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the vertical blur shader object.
    m_VerticalBlurShader = (VerticalBlurShaderClass *)_aligned_malloc(
        sizeof(VerticalBlurShaderClass), 16);
    new (m_VerticalBlurShader) VerticalBlurShaderClass();
    if (!m_VerticalBlurShader) {
      return false;
    }

    // Initialize the vertical blur shader object.
    result = m_VerticalBlurShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the vertical blur shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the up sample render to texture object.
    m_UpSampleTexure =
        (RenderTextureClass *)_aligned_malloc(sizeof(RenderTextureClass), 16);
    new (m_UpSampleTexure) RenderTextureClass();
    if (!m_UpSampleTexure) {
      return false;
    }

    // Initialize the up sample render to texture object.
    result = m_UpSampleTexure->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
                                          SCREEN_DEPTH, 0.1f);
    if (!result) {
      MessageBox(
          hwnd, L"Could not initialize the up sample render to texture object.",
          L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the full screen ortho window object.
    m_FullScreenWindow = new OrthoWindowClass();
    if (!m_FullScreenWindow) {
      return false;
    }

    // Initialize the full screen ortho window object.
    result = m_FullScreenWindow->Initialize(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
    if (!result) {
      MessageBox(hwnd,
                 L"Could not initialize the full screen ortho window object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  {
    // Create the soft shadow shader object.
    m_SoftShadowShader = (SoftShadowShaderClass *)_aligned_malloc(
        sizeof(SoftShadowShaderClass), 16);
    new (m_SoftShadowShader) SoftShadowShaderClass();
    if (!m_SoftShadowShader) {
      return false;
    }

    // Initialize the soft shadow shader object.
    result = m_SoftShadowShader->Initialize(hwnd);
    if (!result) {
      MessageBox(hwnd, L"Could not initialize the soft shadow shader object.",
                 L"Error", MB_OK);
      return false;
    }
  }

  return true;
}

void GraphicsClass::Shutdown() {

  // Release the soft shadow shader object.
  if (m_SoftShadowShader) {
    m_SoftShadowShader->Shutdown();
    m_SoftShadowShader->~SoftShadowShaderClass();
    _aligned_free(m_SoftShadowShader);
    m_SoftShadowShader = 0;
  }

  if (m_FullScreenWindow) {
    m_FullScreenWindow->Shutdown();
    delete m_FullScreenWindow;
    m_FullScreenWindow = 0;
  }

  if (m_UpSampleTexure) {
    m_UpSampleTexure->Shutdown();
    m_UpSampleTexure->~RenderTextureClass();
    _aligned_free(m_UpSampleTexure);
    m_UpSampleTexure = 0;
  }

  if (m_VerticalBlurShader) {
    m_VerticalBlurShader->Shutdown();
    m_VerticalBlurShader->~VerticalBlurShaderClass();
    _aligned_free(m_VerticalBlurShader);
    m_VerticalBlurShader = 0;
  }

  if (m_VerticalBlurTexture) {
    m_VerticalBlurTexture->Shutdown();
    m_VerticalBlurTexture->~RenderTextureClass();
    _aligned_free(m_VerticalBlurTexture);
    m_VerticalBlurTexture = 0;
  }

  if (m_HorizontalBlurShader) {
    m_HorizontalBlurShader->Shutdown();
    m_HorizontalBlurShader->~HorizontalBlurShaderClass();
    _aligned_free(m_HorizontalBlurShader);
    m_HorizontalBlurShader = 0;
  }

  if (m_HorizontalBlurTexture) {
    m_HorizontalBlurTexture->Shutdown();
    m_HorizontalBlurTexture->~RenderTextureClass();
    _aligned_free(m_HorizontalBlurTexture);
    m_HorizontalBlurTexture = 0;
  }

  if (texture_shader_) {
    texture_shader_->Shutdown();
    texture_shader_->~TextureShaderClass();
    _aligned_free(texture_shader_);
    texture_shader_ = 0;
  }

  if (m_SmallWindow) {
    m_SmallWindow->Shutdown();
    delete m_SmallWindow;
    m_SmallWindow = 0;
  }

  if (m_DownSampleTexure) {
    m_DownSampleTexure->Shutdown();
    m_DownSampleTexure->~RenderTextureClass();
    _aligned_free(m_DownSampleTexure);
    m_DownSampleTexure = 0;
  }

  // Release the shadow shader object.
  if (m_ShadowShader) {
    m_ShadowShader->Shutdown();
    m_ShadowShader->~ShadowShaderClass();
    _aligned_free(m_ShadowShader);
    m_ShadowShader = 0;
  }

  // Release the black and white render to texture.
  if (m_BlackWhiteRenderTexture) {
    m_BlackWhiteRenderTexture->Shutdown();
    m_BlackWhiteRenderTexture->~RenderTextureClass();
    _aligned_free(m_BlackWhiteRenderTexture);
    m_BlackWhiteRenderTexture = 0;
  }

  // Release the depth shader object.
  if (depth_shader_) {
    depth_shader_->Shutdown();
    depth_shader_->~DepthShaderClass();
    _aligned_free(depth_shader_);
    depth_shader_ = 0;
  }

  if (render_texture_) {
    render_texture_->Shutdown();
    render_texture_->~RenderTextureClass();
    _aligned_free(render_texture_);
    render_texture_ = 0;
  }

  if (light_) {
    light_->~LightClass();
    _aligned_free(light_);
    light_ = nullptr;
    ;
  }

  // Release the ground model object.
  if (m_GroundModel) {
    m_GroundModel->Shutdown();
    delete m_GroundModel;
    m_GroundModel = 0;
  }

  // Release the sphere model object.
  if (m_SphereModel) {
    m_SphereModel->Shutdown();
    delete m_SphereModel;
    m_SphereModel = 0;
  }

  // Release the cube model object.
  if (m_CubeModel) {
    m_CubeModel->Shutdown();
    delete m_CubeModel;
    m_CubeModel = 0;
  }

  if (camera_) {
    camera_->~Camera();
    _aligned_free(camera_);
    camera_ = nullptr;
  }
}

void GraphicsClass::Frame(float deltatime) {

  static float lightPositionX = -5.0f;

  camera_->SetPosition(pos_x_, pos_y_, pos_z_);
  camera_->SetRotation(rot_x_, rot_y_, rot_z_);

  // Update the position of the light each frame.
  lightPositionX += 0.05f;
  if (lightPositionX > 5.0f) {
    lightPositionX = -5.0f;
  }

  // Update the position of the light.
  light_->SetPosition(lightPositionX, 8.0f, -5.0f);

  // Render the graphics scene.
  Render();
}

bool GraphicsClass::RenderSceneToTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  render_texture_->SetRenderTarget(directx_device_->GetDeviceContext());

  render_texture_->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  light_->GenerateViewMatrix();

  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", lightViewMatrix);
  parameters.SetMatrix("projectionMatrix", lightProjectionMatrix);

  m_CubeModel->Render(*depth_shader_, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_SphereModel->Render(*depth_shader_, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_GroundModel->Render(*depth_shader_, parameters);

  directx_device_->SetBackBufferRenderTarget();

  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderBlackAndWhiteShadows() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_BlackWhiteRenderTexture->SetRenderTarget(
      directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_BlackWhiteRenderTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Generate the light view matrix based on the light's position.
  light_->GenerateViewMatrix();

  // Get the world, view, and projection matrices from the camera and d3d
  // objects.
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  // Get the light's view and projection matrices from the light object.
  XMMATRIX lightViewMatrix, lightProjectionMatrix;
  light_->GetViewMatrix(lightViewMatrix);
  light_->GetProjectionMatrix(lightProjectionMatrix);

  // Setup the world matrix for the cube model.
  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", viewMatrix);
  parameters.SetMatrix("projectionMatrix", projectionMatrix);
  parameters.SetMatrix("lightViewMatrix", lightViewMatrix);
  parameters.SetMatrix("lightProjectionMatrix", lightProjectionMatrix);
  parameters.SetTexture("depthMapTexture",
                        render_texture_->GetShaderResourceView());
  parameters.SetVector3("lightPosition", light_->GetPosition());
  // Render the cube model using the shadow shader.
  m_CubeModel->Render(*m_ShadowShader, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_SphereModel->Render(*m_ShadowShader, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();
  parameters.SetMatrix("worldMatrix", worldMatrix);

  m_GroundModel->Render(*m_ShadowShader, parameters);

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::DownSampleTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  m_DownSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_DownSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions being that it is smaller.
  m_DownSampleTexure->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  // Render the small ortho window using the texture shader and the render to
  // texture of the scene as the texture resource.
  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("baseViewMatrix", baseViewMatrix);
  parameters.SetMatrix("orthoMatrix", orthoMatrix);
  parameters.SetTexture("texture",
                        m_BlackWhiteRenderTexture->GetShaderResourceView());

  auto result =
      texture_shader_->Render(m_SmallWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderHorizontalBlurToTexture() {

  // Store the screen width in a float that will be used in the horizontal blur
  // shader.
  auto screenSizeX = (float)(SHADOWMAP_WIDTH / 2);

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_HorizontalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_HorizontalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_HorizontalBlurTexture->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  // Render the small ortho window using the horizontal blur shader and the down
  // sampled render to texture resource.

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", baseViewMatrix);
  parameters.SetMatrix("projectionMatrix", orthoMatrix);
  parameters.SetFloat("screenWidth", screenSizeX);
  parameters.SetTexture("texture", m_DownSampleTexure->GetShaderResourceView());

  auto result = m_HorizontalBlurShader->Render(m_SmallWindow->GetIndexCount(),
                                               parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::RenderVerticalBlurToTexture() {

  // Store the screen height in a float that will be used in the vertical blur
  // shader.
  auto screenSizeY = (float)(SHADOWMAP_HEIGHT / 2);

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_VerticalBlurTexture->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_VerticalBlurTexture->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_VerticalBlurTexture->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the small ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_SmallWindow->Render(directx_device_->GetDeviceContext());

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", baseViewMatrix);
  parameters.SetMatrix("projectionMatrix", orthoMatrix);
  parameters.SetFloat("screenHeight", screenSizeY);
  parameters.SetTexture("texture",
                        m_HorizontalBlurTexture->GetShaderResourceView());

  // Render the small ortho window using the vertical blur shader and the
  // horizontal blurred render to texture resource.
  auto result =
      m_VerticalBlurShader->Render(m_SmallWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

bool GraphicsClass::UpSampleTexture() {

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Set the render target to be the render to texture.
  m_UpSampleTexure->SetRenderTarget(directx_device_->GetDeviceContext());

  // Clear the render to texture.
  m_UpSampleTexure->ClearRenderTarget(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world and view matrices from the camera and d3d objects.
  XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
  directx_device_->GetWorldMatrix(worldMatrix);
  camera_->GetBaseViewMatrix(baseViewMatrix);

  // Get the ortho matrix from the render to texture since texture has different
  // dimensions.
  m_UpSampleTexure->GetOrthoMatrix(orthoMatrix);

  // Turn off the Z buffer to begin all 2D rendering.
  directx_device_->TurnZBufferOff();

  // Put the full screen ortho window vertex and index buffers on the graphics
  // pipeline to prepare them for drawing.
  m_FullScreenWindow->Render(directx_device_->GetDeviceContext());

  // Render the full screen ortho window using the texture shader and the small
  // sized final blurred render to texture resource.

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("baseViewMatrix", baseViewMatrix);
  parameters.SetMatrix("orthoMatrix", orthoMatrix);
  parameters.SetTexture("texture",
                        m_VerticalBlurTexture->GetShaderResourceView());

  auto result =
      texture_shader_->Render(m_FullScreenWindow->GetIndexCount(), parameters);
  if (!result) {
    return false;
  }

  // Turn the Z buffer back on now that all 2D rendering has completed.
  directx_device_->TurnZBufferOn();

  // Reset the render target back to the original back buffer and not the render
  // to texture anymore.
  directx_device_->SetBackBufferRenderTarget();

  // Reset the viewport back to the original.
  directx_device_->ResetViewport();

  return true;
}

void GraphicsClass::Render() {

  // First render the scene to a texture.
  auto result = RenderSceneToTexture();
  if (!result) {
    return;
  }

  // Next render the shadowed scene in black and white.
  result = RenderBlackAndWhiteShadows();
  if (!result) {
    return;
  }

  // Then down sample the black and white scene texture.
  result = DownSampleTexture();
  if (!result) {
    return;
  }

  // Perform a horizontal blur on the down sampled texture.
  result = RenderHorizontalBlurToTexture();
  if (!result) {
    return;
  }

  // Now perform a vertical blur on the texture.
  result = RenderVerticalBlurToTexture();
  if (!result) {
    return;
  }

  // Finally up sample the final blurred render to texture that can now be used
  // in the soft shadow shader.
  result = UpSampleTexture();
  if (!result) {
    return;
  }

  auto directx_device_ = DirectX11Device::GetD3d11DeviceInstance();

  // Clear the buffers to begin the scene.
  directx_device_->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

  // Generate the view matrix based on the camera's position.
  camera_->Render();

  // Get the world, view, and projection matrices from the camera and d3d
  // objects.
  XMMATRIX viewMatrix, projectionMatrix;
  camera_->GetViewMatrix(viewMatrix);
  directx_device_->GetProjectionMatrix(projectionMatrix);

  // Setup the world matrix for the cube model.
  auto worldMatrix = m_CubeModel->GetWorldMatrix();

  ShaderParameterContainer parameters;
  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetMatrix("viewMatrix", viewMatrix);
  parameters.SetMatrix("projectionMatrix", projectionMatrix);
  parameters.SetTexture("shadowTexture",
                        m_UpSampleTexure->GetShaderResourceView());
  parameters.SetVector3("lightPosition", light_->GetPosition());
  parameters.SetTexture("texture", m_CubeModel->GetTexture());
  parameters.SetVector4("diffuseColor", light_->GetDiffuseColor());
  parameters.SetVector4("ambientColor", light_->GetAmbientColor());

  // Render the cube model using the soft shadow shader.
  m_CubeModel->Render(*m_SoftShadowShader, parameters);

  worldMatrix = m_SphereModel->GetWorldMatrix();

  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetTexture("texture", m_SphereModel->GetTexture());

  m_SphereModel->Render(*m_SoftShadowShader, parameters);

  worldMatrix = m_GroundModel->GetWorldMatrix();

  parameters.SetMatrix("worldMatrix", worldMatrix);
  parameters.SetTexture("texture", m_GroundModel->GetTexture());

  m_GroundModel->Render(*m_SoftShadowShader, parameters);

  result =
      m_SoftShadowShader->Render(m_GroundModel->GetIndexCount(), parameters);
  if (!result) {
    return;
  }

  // Present the rendered scene to the screen.
  directx_device_->EndScene();
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\horizontalblurshaderclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "horizontalblurshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool HorizontalBlurShaderClass::Initialize(HWND hwnd) {
  bool result;

  result =
      InitializeShader(hwnd, L"./horizontalblur.vs", L"./horizontalblur.ps");
  if (!result) {
    return false;
  }

  return true;
}

void HorizontalBlurShaderClass::Shutdown() { ShutdownShader(); }

bool HorizontalBlurShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto screenWidth = parameters.GetFloat("screenWidth");
  auto texture = parameters.GetTexture("texture");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    texture, screenWidth);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool HorizontalBlurShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                                 WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
  unsigned int numElements;
  D3D11_SAMPLER_DESC samplerDesc;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_BUFFER_DESC screenSizeBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(
      vsFilename, NULL, NULL, "HorizontalBlurVertexShader", "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(
      psFilename, NULL, NULL, "HorizontalBlurPixelShader", "ps_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  polygonLayout[1].SemanticName = "TEXCOORD";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  polygonLayout[1].InputSlot = 0;
  polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[1].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
  screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  screenSizeBufferDesc.MiscFlags = 0;
  screenSizeBufferDesc.StructureByteStride = 0;

  result =
      device->CreateBuffer(&screenSizeBufferDesc, NULL, &screen_size_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void HorizontalBlurShaderClass::ShutdownShader() {}

void HorizontalBlurShaderClass::OutputShaderErrorMessage(
    ID3D10Blob *errorMessage, HWND hwnd, WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool HorizontalBlurShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    float screenWidth) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  MatrixBufferType *dataPtr;
  unsigned int buffer_number;
  ScreenSizeBufferType *dataPtr2;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  result = device_context->Map(screen_size_buffer_.Get(), 0,
                               D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr2 = (ScreenSizeBufferType *)mappedResource.pData;

  dataPtr2->screenWidth = screenWidth;
  dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

  device_context->Unmap(screen_size_buffer_.Get(), 0);

  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       screen_size_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &texture);

  return true;
}

void HorizontalBlurShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\modelclass.cpp
#include "../CommonFramework/DirectX11Device.h"
#include "modelclass.h"

#include "../CommonFramework/DirectX11Device.h"
#include "IShader.h"
#include "ShaderParameterContainer.h"
#include "modelclass.h"

#include <DirectXMath.h>
#include <fstream>

ModelClass::~ModelClass() { Shutdown(); }

bool ModelClass::Initialize(const std::string &modelFilename,
                            const std::wstring &textureFilename) {
  if (!LoadModel(modelFilename) || !InitializeBuffers() ||
      !LoadTexture(textureFilename)) {
    return false;
  }
  return true;
}

void ModelClass::Shutdown() {
  ReleaseTexture();
  ShutdownBuffers();
  ReleaseModel();
}

void ModelClass::Render(
    const IShader &shader,
    const ShaderParameterContainer &parameterContainer) const {

  RenderBuffers();

  shader.Render(GetIndexCount(), parameterContainer);
}

ID3D11ShaderResourceView *ModelClass::GetTexture() const {
  return texture_ ? texture_->GetTexture() : nullptr;
}

bool ModelClass::InitializeBuffers() {
  std::vector<Vertex> vertices(vertex_count_);
  std::vector<unsigned long> indices(index_count_);

  for (int i = 0; i < vertex_count_; i++) {
    vertices[i].position =
        DirectX::XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
    vertices[i].texture = DirectX::XMFLOAT2(model_[i].tu, model_[i].tv);
    vertices[i].normal =
        DirectX::XMFLOAT3(model_[i].nx, model_[i].ny, model_[i].nz);
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

void ModelClass::ShutdownBuffers() {
  index_buffer_.Reset();
  vertex_buffer_.Reset();
}

void ModelClass::RenderBuffers() const {
  UINT stride = sizeof(Vertex);
  UINT offset = 0;

  auto deviceContext =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();
  deviceContext->IASetVertexBuffers(0, 1, vertex_buffer_.GetAddressOf(),
                                    &stride, &offset);
  deviceContext->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool ModelClass::LoadTexture(const std::wstring &filename) {
  texture_ = std::make_unique<TextureClass>();
  return texture_->Initialize(filename.c_str());
}

void ModelClass::ReleaseTexture() {
  if (texture_) {
    texture_->Shutdown();
    texture_.reset();
  }
}

bool ModelClass::LoadModel(const std::string &filename) {
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

void ModelClass::ReleaseModel() { model_.clear(); }

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\orthowindowclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "orthowindowclass.h"

OrthoWindowClass::OrthoWindowClass() {
  vertex_buffer_ = nullptr;
  index_buffer_ = nullptr;
}

OrthoWindowClass::OrthoWindowClass(const OrthoWindowClass &other) {}

OrthoWindowClass::~OrthoWindowClass() {}

bool OrthoWindowClass::Initialize(int windowWidth, int windowHeight) {
  bool result;

  result = InitializeBuffers(windowWidth, windowHeight);
  if (!result) {
    return false;
  }

  return true;
}

void OrthoWindowClass::Shutdown() { ShutdownBuffers(); }

void OrthoWindowClass::Render(ID3D11DeviceContext *device_context) {

  RenderBuffers(device_context);
}

int OrthoWindowClass::GetIndexCount() { return index_count_; }

bool OrthoWindowClass::InitializeBuffers(int windowWidth, int windowHeight) {
  float left, right, top, bottom;

  D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
  D3D11_SUBRESOURCE_DATA vertex_data, indexData;
  HRESULT result;
  int i;

  // Calculate the screen coordinates of the left side of the window.
  left = (float)((windowWidth / 2) * -1);

  // Calculate the screen coordinates of the right side of the window.
  right = left + (float)windowWidth;

  // Calculate the screen coordinates of the top of the window.
  top = (float)(windowHeight / 2);

  // Calculate the screen coordinates of the bottom of the window.
  bottom = top - (float)windowHeight;

  vertex_count_ = 6;

  index_count_ = vertex_count_;

  auto vertices = new VertexType[vertex_count_];
  if (!vertices) {
    return false;
  }

  auto indices = new unsigned long[index_count_];
  if (!indices) {
    return false;
  }

  // Load the vertex array with data.
  // First triangle.
  vertices[0].position = XMFLOAT3(left, top, 0.0f); // Top left.
  vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

  vertices[1].position = XMFLOAT3(right, bottom, 0.0f); // Bottom right.
  vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

  vertices[2].position = XMFLOAT3(left, bottom, 0.0f); // Bottom left.
  vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

  // Second triangle.
  vertices[3].position = XMFLOAT3(left, top, 0.0f); // Top left.
  vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

  vertices[4].position = XMFLOAT3(right, top, 0.0f); // Top right.
  vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

  vertices[5].position = XMFLOAT3(right, bottom, 0.0f); // Bottom right.
  vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

  for (i = 0; i < index_count_; i++) {
    indices[i] = i;
  }

  vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
  vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_buffer_desc.CPUAccessFlags = 0;
  vertex_buffer_desc.MiscFlags = 0;
  vertex_buffer_desc.StructureByteStride = 0;

  vertex_data.pSysMem = vertices;
  vertex_data.SysMemPitch = 0;
  vertex_data.SysMemSlicePitch = 0;

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result =
      device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_);
  if (FAILED(result)) {
    return false;
  }

  index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
  index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  index_buffer_desc.CPUAccessFlags = 0;
  index_buffer_desc.MiscFlags = 0;
  index_buffer_desc.StructureByteStride = 0;

  indexData.pSysMem = indices;
  indexData.SysMemPitch = 0;
  indexData.SysMemSlicePitch = 0;

  result = device->CreateBuffer(&index_buffer_desc, &indexData, &index_buffer_);
  if (FAILED(result)) {
    return false;
  }

  delete[] vertices;
  vertices = 0;

  delete[] indices;
  indices = 0;

  return true;
}

void OrthoWindowClass::ShutdownBuffers() {

  if (index_buffer_) {
    index_buffer_->Release();
    index_buffer_ = nullptr;
  }

  if (vertex_buffer_) {
    vertex_buffer_->Release();
    vertex_buffer_ = nullptr;
  }
}

void OrthoWindowClass::RenderBuffers(ID3D11DeviceContext *device_context) {
  unsigned int stride;
  unsigned int offset;

  stride = sizeof(VertexType);
  offset = 0;

  device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

  device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

  device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\positionclass.cpp

#include "positionclass.h"

PositionClass::PositionClass() {
  position_x_ = 0.0f;
  position_y_ = 0.0f;
  position_z_ = 0.0f;

  rotation_x_ = 0.0f;
  rotation_y_ = 0.0f;
  rotation_z_ = 0.0f;

  frame_time_ = 0.0f;

  m_forwardSpeed = 0.0f;
  m_backwardSpeed = 0.0f;
  m_upwardSpeed = 0.0f;
  m_downwardSpeed = 0.0f;
  left_turning_speed_ = 0.0f;
  right_turning_speed_ = 0.0f;
  m_lookUpSpeed = 0.0f;
  m_lookDownSpeed = 0.0f;
}

PositionClass::PositionClass(const PositionClass &other) {}

PositionClass::~PositionClass() {}

void PositionClass::SetPosition(float x, float y, float z) {
  position_x_ = x;
  position_y_ = y;
  position_z_ = z;
}

void PositionClass::SetRotation(float x, float y, float z) {
  rotation_x_ = x;
  rotation_y_ = y;
  rotation_z_ = z;
}

void PositionClass::GetPosition(float &x, float &y, float &z) {
  x = position_x_;
  y = position_y_;
  z = position_z_;
}

void PositionClass::GetRotation(float &x, float &y, float &z) {
  x = rotation_x_;
  y = rotation_y_;
  z = rotation_z_;
}

void PositionClass::SetFrameTime(float time) { frame_time_ = time; }

void PositionClass::MoveForward(bool keydown) {
  float radians;

  // Update the forward speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    m_forwardSpeed += frame_time_ * 0.001f;

    if (m_forwardSpeed > (frame_time_ * 0.03f)) {
      m_forwardSpeed = frame_time_ * 0.03f;
    }
  } else {
    m_forwardSpeed -= frame_time_ * 0.0007f;

    if (m_forwardSpeed < 0.0f) {
      m_forwardSpeed = 0.0f;
    }
  }

  // Convert degrees to radians.
  radians = rotation_y_ * 0.0174532925f;

  // Update the position.
  position_x_ += sinf(radians) * m_forwardSpeed;
  position_z_ += cosf(radians) * m_forwardSpeed;
}

void PositionClass::MoveBackward(bool keydown) {
  float radians;

  // Update the backward speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    m_backwardSpeed += frame_time_ * 0.001f;

    if (m_backwardSpeed > (frame_time_ * 0.03f)) {
      m_backwardSpeed = frame_time_ * 0.03f;
    }
  } else {
    m_backwardSpeed -= frame_time_ * 0.0007f;

    if (m_backwardSpeed < 0.0f) {
      m_backwardSpeed = 0.0f;
    }
  }

  // Convert degrees to radians.
  radians = rotation_y_ * 0.0174532925f;

  // Update the position.
  position_x_ -= sinf(radians) * m_backwardSpeed;
  position_z_ -= cosf(radians) * m_backwardSpeed;
}

void PositionClass::MoveUpward(bool keydown) {
  // Update the upward speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    m_upwardSpeed += frame_time_ * 0.003f;

    if (m_upwardSpeed > (frame_time_ * 0.03f)) {
      m_upwardSpeed = frame_time_ * 0.03f;
    }
  } else {
    m_upwardSpeed -= frame_time_ * 0.002f;

    if (m_upwardSpeed < 0.0f) {
      m_upwardSpeed = 0.0f;
    }
  }

  // Update the height position.
  position_y_ += m_upwardSpeed;
}

void PositionClass::MoveDownward(bool keydown) {
  // Update the downward speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    m_downwardSpeed += frame_time_ * 0.003f;

    if (m_downwardSpeed > (frame_time_ * 0.03f)) {
      m_downwardSpeed = frame_time_ * 0.03f;
    }
  } else {
    m_downwardSpeed -= frame_time_ * 0.002f;

    if (m_downwardSpeed < 0.0f) {
      m_downwardSpeed = 0.0f;
    }
  }

  // Update the height position.
  position_y_ -= m_downwardSpeed;
}

void PositionClass::TurnLeft(bool keydown) {
  // Update the left turn speed movement based on the frame time and whether the
  // user is holding the key down or not.
  if (keydown) {
    left_turning_speed_ += frame_time_ * 0.01f;

    if (left_turning_speed_ > (frame_time_ * 0.15f)) {
      left_turning_speed_ = frame_time_ * 0.15f;
    }
  } else {
    left_turning_speed_ -= frame_time_ * 0.005f;

    if (left_turning_speed_ < 0.0f) {
      left_turning_speed_ = 0.0f;
    }
  }

  // Update the rotation_.
  rotation_y_ -= left_turning_speed_;

  // Keep the rotation_ in the 0 to 360 range.
  if (rotation_y_ < 0.0f) {
    rotation_y_ += 360.0f;
  }
}

void PositionClass::TurnRight(bool keydown) {
  // Update the right turn speed movement based on the frame time and whether
  // the user is holding the key down or not.
  if (keydown) {
    right_turning_speed_ += frame_time_ * 0.01f;

    if (right_turning_speed_ > (frame_time_ * 0.15f)) {
      right_turning_speed_ = frame_time_ * 0.15f;
    }
  } else {
    right_turning_speed_ -= frame_time_ * 0.005f;

    if (right_turning_speed_ < 0.0f) {
      right_turning_speed_ = 0.0f;
    }
  }

  // Update the rotation_.
  rotation_y_ += right_turning_speed_;

  // Keep the rotation_ in the 0 to 360 range.
  if (rotation_y_ > 360.0f) {
    rotation_y_ -= 360.0f;
  }
}

void PositionClass::LookUpward(bool keydown) {
  // Update the upward rotation_ speed movement based on the frame time and
  // whether the user is holding the key down or not.
  if (keydown) {
    m_lookUpSpeed += frame_time_ * 0.01f;

    if (m_lookUpSpeed > (frame_time_ * 0.15f)) {
      m_lookUpSpeed = frame_time_ * 0.15f;
    }
  } else {
    m_lookUpSpeed -= frame_time_ * 0.005f;

    if (m_lookUpSpeed < 0.0f) {
      m_lookUpSpeed = 0.0f;
    }
  }

  // Update the rotation_.
  rotation_x_ -= m_lookUpSpeed;

  // Keep the rotation_ maximum 90 degrees.
  if (rotation_x_ > 90.0f) {
    rotation_x_ = 90.0f;
  }
}

void PositionClass::LookDownward(bool keydown) {
  // Update the downward rotation_ speed movement based on the frame time and
  // whether the user is holding the key down or not.
  if (keydown) {
    m_lookDownSpeed += frame_time_ * 0.01f;

    if (m_lookDownSpeed > (frame_time_ * 0.15f)) {
      m_lookDownSpeed = frame_time_ * 0.15f;
    }
  } else {
    m_lookDownSpeed -= frame_time_ * 0.005f;

    if (m_lookDownSpeed < 0.0f) {
      m_lookDownSpeed = 0.0f;
    }
  }

  // Update the rotation_.
  rotation_x_ += m_lookDownSpeed;

  // Keep the rotation_ maximum 90 degrees.
  if (rotation_x_ < -90.0f) {
    rotation_x_ = -90.0f;
  }
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\RenderPipeline.cpp

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\RenderSystem.cpp

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\rendertextureclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "rendertextureclass.h"

RenderTextureClass::RenderTextureClass() {
  render_target_texture_ = 0;
  render_target_view_ = 0;
  shader_resource_view_ = 0;
  depth_stencil_buffer_ = 0;
  depth_stencil_view_ = 0;
}

RenderTextureClass::RenderTextureClass(const RenderTextureClass &other) {}

RenderTextureClass::~RenderTextureClass() {}

bool RenderTextureClass::Initialize(int textureWidth, int textureHeight,
                                    float screenDepth, float screenNear) {
  D3D11_TEXTURE2D_DESC textureDesc;
  HRESULT result;
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
  D3D11_TEXTURE2D_DESC depthBufferDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

  ZeroMemory(&textureDesc, sizeof(textureDesc));

  textureDesc.Width = textureWidth;
  textureDesc.Height = textureHeight;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateTexture2D(&textureDesc, NULL, &render_target_texture_);
  if (FAILED(result)) {
    return false;
  }

  renderTargetViewDesc.Format = textureDesc.Format;
  renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  renderTargetViewDesc.Texture2D.MipSlice = 0;

  result = device->CreateRenderTargetView(
      render_target_texture_, &renderTargetViewDesc, &render_target_view_);
  if (FAILED(result)) {
    return false;
  }

  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

  result = device->CreateShaderResourceView(
      render_target_texture_, &shaderResourceViewDesc, &shader_resource_view_);
  if (FAILED(result)) {
    return false;
  }

  ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

  depthBufferDesc.Width = textureWidth;
  depthBufferDesc.Height = textureHeight;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  result =
      device->CreateTexture2D(&depthBufferDesc, NULL, &depth_stencil_buffer_);
  if (FAILED(result)) {
    return false;
  }

  // Initailze the depth stencil view description.
  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  result = device->CreateDepthStencilView(
      depth_stencil_buffer_, &depthStencilViewDesc, &depth_stencil_view_);
  if (FAILED(result)) {
    return false;
  }

  viewport_.Width = (float)textureWidth;
  viewport_.Height = (float)textureHeight;
  viewport_.MinDepth = 0.0f;
  viewport_.MaxDepth = 1.0f;
  viewport_.TopLeftX = 0.0f;
  viewport_.TopLeftY = 0.0f;

  projection_matrix_ = XMMatrixPerspectiveFovLH(
      ((float)XM_PI / 4.0f), ((float)textureWidth / (float)textureHeight),
      screenNear, screenDepth);

  ortho_matrix_ = XMMatrixOrthographicLH(
      (float)textureWidth, (float)textureHeight, screenNear, screenDepth);

  return true;
}

void RenderTextureClass::Shutdown() {
  if (depth_stencil_view_) {
    depth_stencil_view_->Release();
    depth_stencil_view_ = 0;
  }

  if (depth_stencil_buffer_) {
    depth_stencil_buffer_->Release();
    depth_stencil_buffer_ = 0;
  }

  if (shader_resource_view_) {
    shader_resource_view_->Release();
    shader_resource_view_ = 0;
  }

  if (render_target_view_) {
    render_target_view_->Release();
    render_target_view_ = 0;
  }

  if (render_target_texture_) {
    render_target_texture_->Release();
    render_target_texture_ = 0;
  }
}

void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext *device_context) {

  device_context->OMSetRenderTargets(1, &render_target_view_,
                                     depth_stencil_view_);

  device_context->RSSetViewports(1, &viewport_);
}

void RenderTextureClass::ClearRenderTarget(float red, float green, float blue,
                                           float alpha) {
  float color[4];

  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH,
                                        1.0f, 0);
}

ID3D11ShaderResourceView *RenderTextureClass::GetShaderResourceView() {
  return shader_resource_view_;
}

void RenderTextureClass::GetProjectionMatrix(XMMATRIX &projectionMatrix) {
  projectionMatrix = projection_matrix_;
}

void RenderTextureClass::GetOrthoMatrix(XMMATRIX &orthoMatrix) {
  orthoMatrix = ortho_matrix_;
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\ShaderParameterContainer.cpp
#include "ShaderParameterContainer.h"

#include <stdexcept>

void ShaderParameterContainer::SetFloat(const std::string &name, float f) {
  parameters_[name] = f;
}

void ShaderParameterContainer::SetMatrix(const std::string &name,
                                         const DirectX::XMMATRIX &matrix) {
  parameters_[name] = matrix;
}

void ShaderParameterContainer::SetVector3(const std::string &name,
                                          const DirectX::XMFLOAT3 &vector) {
  parameters_[name] = vector;
}

void ShaderParameterContainer::SetVector4(const std::string &name,
                                          const DirectX::XMFLOAT4 &vector) {
  parameters_[name] = vector;
}

void ShaderParameterContainer::SetTexture(const std::string &name,
                                          ID3D11ShaderResourceView *texture) {
  parameters_[name] = texture;
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\shadowshaderclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "ShaderParameterContainer.h"
#include "shadowshaderclass.h"

using namespace DirectX;

bool ShadowShaderClass::Initialize(HWND hwnd) {
  return InitializeShader(hwnd, L"./shadow.vs", L"./shadow.ps");
}

void ShadowShaderClass::Shutdown() { ShutdownShader(); }

bool ShadowShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto lightViewMatrix = parameters.GetMatrix("lightViewMatrix");
  auto lightProjectionMatrix = parameters.GetMatrix("lightProjectionMatrix");
  auto depthMapTexture = parameters.GetTexture("depthMapTexture");
  auto lightPosition = parameters.GetVector3("lightPosition");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    lightViewMatrix, lightProjectionMatrix,
                                    depthMapTexture, lightPosition);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool ShadowShaderClass::InitializeShader(HWND hwnd,
                                         const std::wstring &vsFilename,
                                         const std::wstring &psFilename) {
  HRESULT result;
  Microsoft::WRL::ComPtr<ID3D10Blob> errorMessage;
  Microsoft::WRL::ComPtr<ID3D10Blob> vertexShaderBuffer;
  Microsoft::WRL::ComPtr<ID3D10Blob> pixelShaderBuffer;

  // Compile the vertex shader
  result = D3DCompileFromFile(
      vsFilename.c_str(), nullptr, nullptr, "ShadowVertexShader", "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, vsFilename);
    } else {
      MessageBox(hwnd, vsFilename.c_str(), L"Missing Shader File", MB_OK);
    }
    return false;
  }

  // Compile the pixel shader
  result = D3DCompileFromFile(
      psFilename.c_str(), nullptr, nullptr, "ShadowPixelShader", "ps_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {
    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage.Get(), hwnd, psFilename);
    } else {
      MessageBox(hwnd, psFilename.c_str(), L"Missing Shader File", MB_OK);
    }
    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  // Create the vertex shader
  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(),
                                      nullptr, &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  // Create the pixel shader
  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(),
                                     nullptr, &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  // Create the vertex input layout description
  D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
       D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};

  UINT numElements = ARRAYSIZE(polygonLayout);

  // Create the vertex input layout
  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  // Create a constant buffer description
  D3D11_BUFFER_DESC matrixBufferDesc = {};
  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  // Create the constant buffer pointer
  result = device->CreateBuffer(&matrixBufferDesc, nullptr, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  // Create a texture sampler state description
  D3D11_SAMPLER_DESC samplerDesc = {};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  // Create the texture sampler state
  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

  D3D11_BUFFER_DESC lightBufferDesc2;

  // Setup the description of the light dynamic constant buffer that is in the
  // vertex shader.
  lightBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc2.ByteWidth = sizeof(LightBufferType2);
  lightBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc2.MiscFlags = 0;
  lightBufferDesc2.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc2, NULL, &light_buffee_2);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void ShadowShaderClass::ShutdownShader() {
  sample_state_.Reset();
  matrix_buffer_.Reset();
  layout_.Reset();
  pixel_shader_.Reset();
  vertex_shader_.Reset();
}

void ShadowShaderClass::OutputShaderErrorMessage(
    ID3D10Blob *errorMessage, HWND hwnd, const std::wstring &shaderFilename) {
  char *compileErrors = static_cast<char *>(errorMessage->GetBufferPointer());
  SIZE_T bufferSize = errorMessage->GetBufferSize();

  std::ofstream fout("shader-error.txt");
  for (SIZE_T i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }
  fout.close();

  MessageBox(hwnd,
             L"Error compiling shader. Check shader-error.txt for message.",
             shaderFilename.c_str(), MB_OK);
}

bool ShadowShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, const XMMATRIX &lightViewMatrix,
    const XMMATRIX &lightProjectionMatrix,
    ID3D11ShaderResourceView *depthMapTexture,
    const XMFLOAT3 &lightPosition) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;
  LightBufferType2 *dataPtr3;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;
  XMMATRIX lightViewMatrixCopy = lightViewMatrix;
  XMMATRIX lightProjectionMatrixCopy = lightProjectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);
  lightViewMatrixCopy = XMMatrixTranspose(lightViewMatrix);
  lightProjectionMatrixCopy = XMMatrixTranspose(lightProjectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  auto result = device_context->Map(
      matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;
  dataPtr->lightView = lightViewMatrixCopy;
  dataPtr->lightProjection = lightProjectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &depthMapTexture);

  // Lock the second light constant buffer so it can be written to.
  result = device_context->Map(light_buffee_2.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr3 = (LightBufferType2 *)mappedResource.pData;

  dataPtr3->lightPosition = lightPosition;
  dataPtr3->padding = 0.0f;

  device_context->Unmap(light_buffee_2.Get(), 0);

  // Set the position of the light constant buffer in the vertex shader.
  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       light_buffee_2.GetAddressOf());

  return true;
}

void ShadowShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  // Set the vertex input layout
  device_context->IASetInputLayout(layout_.Get());

  // Set the vertex and pixel shaders that will be used to render this triangle
  device_context->VSSetShader(vertex_shader_.Get(), nullptr, 0);
  device_context->PSSetShader(pixel_shader_.Get(), nullptr, 0);

  // Set the sampler state in the pixel shader
  device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

  // Render the triangle
  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\softshadowshaderclass.cpp

#include "softshadowshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

#include "../CommonFramework/DirectX11Device.h"

using namespace std;
using namespace DirectX;

bool SoftShadowShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"./softshadow.vs", L"./softshadow.ps");
  if (!result) {
    return false;
  }

  return true;
}

void SoftShadowShaderClass::Shutdown() { ShutdownShader(); }

bool SoftShadowShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto shadowTexture = parameters.GetTexture("shadowTexture");
  auto lightPosition = parameters.GetVector3("lightPosition");
  auto texture = parameters.GetTexture("texture");
  auto diffuseColor = parameters.GetVector4("diffuseColor");
  auto ambientColor = parameters.GetVector4("ambientColor");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    texture, shadowTexture, lightPosition,
                                    ambientColor, diffuseColor);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool SoftShadowShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                             WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
  unsigned int numElements;
  D3D11_SAMPLER_DESC samplerDesc;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_BUFFER_DESC lightBufferDesc;
  D3D11_BUFFER_DESC lightBufferDesc2;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "SoftShadowVertexShader",
                              "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(psFilename, NULL, NULL, "SoftShadowPixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      &vertex_shader_);
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     &pixel_shader_);
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  polygonLayout[1].SemanticName = "TEXCOORD";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  polygonLayout[1].InputSlot = 0;
  polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[1].InstanceDataStepRate = 0;

  polygonLayout[2].SemanticName = "NORMAL";
  polygonLayout[2].SemanticIndex = 0;
  polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[2].InputSlot = 0;
  polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[2].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  // Create a wrap texture sampler state description.
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  result = device->CreateSamplerState(&samplerDesc, &m_sampleStateWrap);
  if (FAILED(result)) {
    return false;
  }

  // Create a clamp texture sampler state description.
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

  result = device->CreateSamplerState(&samplerDesc, &m_sampleStateClamp);
  if (FAILED(result)) {
    return false;
  }

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc.ByteWidth = sizeof(LightBufferType);
  lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc.MiscFlags = 0;
  lightBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc, NULL, &light_buffer_);
  if (FAILED(result)) {
    return false;
  }

  // Setup the description of the light dynamic constant buffer that is in the
  // vertex shader.
  lightBufferDesc2.Usage = D3D11_USAGE_DYNAMIC;
  lightBufferDesc2.ByteWidth = sizeof(LightBufferType2);
  lightBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lightBufferDesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  lightBufferDesc2.MiscFlags = 0;
  lightBufferDesc2.StructureByteStride = 0;

  result = device->CreateBuffer(&lightBufferDesc2, NULL, &m_lightBuffer2);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void SoftShadowShaderClass::ShutdownShader() {}

void SoftShadowShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                                     HWND hwnd,
                                                     WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool SoftShadowShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    ID3D11ShaderResourceView *shadowTexture, const XMFLOAT3 &lightPosition,
    const XMFLOAT4 &ambientColor, const XMFLOAT4 &diffuseColor) const {

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  unsigned int buffer_number;
  MatrixBufferType *dataPtr;
  LightBufferType *dataPtr2;
  LightBufferType2 *dataPtr3;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  auto result = device_context->Map(
      matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &texture);
  device_context->PSSetShaderResources(1, 1, &shadowTexture);

  result = device_context->Map(light_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr2 = (LightBufferType *)mappedResource.pData;

  dataPtr2->ambientColor = ambientColor;
  dataPtr2->diffuseColor = diffuseColor;

  device_context->Unmap(light_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->PSSetConstantBuffers(buffer_number, 1,
                                       light_buffer_.GetAddressOf());

  // Lock the second light constant buffer so it can be written to.

  result = device_context->Map(m_lightBuffer2.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr3 = (LightBufferType2 *)mappedResource.pData;

  dataPtr3->lightPosition = lightPosition;
  dataPtr3->padding = 0.0f;

  device_context->Unmap(m_lightBuffer2.Get(), 0);

  // Set the position of the light constant buffer in the vertex shader.
  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       m_lightBuffer2.GetAddressOf());

  return true;
}

void SoftShadowShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  // Set the sampler states in the pixel shader.
  device_context->PSSetSamplers(0, 1, m_sampleStateClamp.GetAddressOf());
  device_context->PSSetSamplers(1, 1, m_sampleStateWrap.GetAddressOf());

  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\System.cpp
#include "System.h"

#include "../CommonFramework/Input.h"
#include "../CommonFramework/Timer.h"

#include "Graphics.h"
#include "positionclass.h"

System::System() {}

System::~System() {}

bool System::Initialize() {

  ApplicationInstance = this;
  SetWindProc(WndProc);

  SystemBase::Initialize();

  unsigned int screenWidth = 800, screenHeight = 600;

  // GetScreenWidthAndHeight(screenWidth, screenHeight);

  {
    graphics_ = new GraphicsClass();
    if (!graphics_) {
      return false;
    }
    bool result = graphics_->Initialize(screenWidth, screenHeight,
                                        GetApplicationHandle());
    if (!result) {
      return false;
    }
  }

  {
    position_ = new PositionClass();
    if (!position_) {
      return false;
    }
    // Set the initial position of the viewer to the same as the initial camera
    // position.
    position_->SetPosition(0.0f, 2.0f, -10.0f);
  }

  return true;
}

bool System::Frame() {

  SystemBase::Frame();

  bool result;

  result = GetInputComponent().Frame();
  if (result == false) {
    return false;
  }

  if (GetInputComponent().IsEscapePressed() == true) {
    return true;
  }

  result = HandleInput(GetTimerComponent().GetTime());
  if (!result) {
    return false;
  }

  float posX, posY, posZ;
  float rotX, rotY, rotZ;
  position_->GetPosition(posX, posY, posZ);
  position_->GetRotation(rotX, rotY, rotZ);

  graphics_->SetPosition(posX, posY, posZ);
  graphics_->SetRotation(rotX, rotY, rotZ);

  graphics_->Frame(0.0f);
  if (!result) {
    return false;
  }

  auto delta_time = GetTimerComponent().GetTime();

  graphics_->Render();
  if (!result) {
    return false;
  }

  return true;
}

bool System::HandleInput(float frameTime) {

  bool keyDown;

  position_->SetFrameTime(frameTime);

  keyDown = GetInputComponent().IsLeftPressed();
  position_->TurnLeft(keyDown);

  keyDown = GetInputComponent().IsRightPressed();
  position_->TurnRight(keyDown);

  keyDown = GetInputComponent().IsUpPressed();
  position_->MoveForward(keyDown);

  keyDown = GetInputComponent().IsDownPressed();
  position_->MoveBackward(keyDown);

  keyDown = GetInputComponent().IsAPressed();
  position_->MoveUpward(keyDown);

  keyDown = GetInputComponent().IsZPressed();
  position_->MoveDownward(keyDown);

  keyDown = GetInputComponent().IsPgUpPressed();
  position_->LookUpward(keyDown);

  keyDown = GetInputComponent().IsPgDownPressed();
  position_->LookDownward(keyDown);

  return true;
}

void System::Shutdown() {

  SystemBase::Shutdown();

  if (graphics_) {
    graphics_->Shutdown();
    delete graphics_;
    graphics_ = 0;
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam,
                         LPARAM lparam) {

  switch (umessage) {
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }

  case WM_CLOSE: {
    PostQuitMessage(0);
    return 0;
  }

  default: {
    return ApplicationInstance->MessageHandler(hwnd, umessage, wparam, lparam);
  }
  }
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\textureclass.cpp
//
//
// #include "textureclass.h"
// #include "../CommonFramework/DirectX11Device.h"
//
// TextureClass::TextureClass() { texture_ = nullptr; }
//
// TextureClass::TextureClass(const TextureClass &other) {}
//
// TextureClass::~TextureClass() {}
//
// bool TextureClass::Initialize(WCHAR *filename) {
//  HRESULT result;
//
//  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
//
//  result = CreateDDSTextureFromFile(device, filename, NULL, &texture_);
//  if (FAILED(result)) {
//    return false;
//  }
//
//  return true;
//}
//
// void TextureClass::Shutdown() {
//
//  if (texture_) {
//    texture_->Release();
//    texture_ = nullptr;
//  }
//}
//
// ID3D11ShaderResourceView *TextureClass::GetTexture() { return texture_; }

#include "TextureClass.h"

#include "../CommonFramework/DirectX11Device.h"

#include <DDSTextureLoader.h>
#include <d3d11.h>
#include <stdexcept>

bool TextureClass::Initialize(const WCHAR *filename) {
  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();
  auto result = DirectX::CreateDDSTextureFromFile(device, filename, nullptr,
                                                  texture_.GetAddressOf());
  if (FAILED(result)) {
    throw std::runtime_error("Failed to create texture from file");
  }
  return true;
}

void TextureClass::Shutdown() { texture_.Reset(); }

ID3D11ShaderResourceView *TextureClass::GetTexture() const {
  return texture_.Get();
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\textureshaderclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "ShaderParameterContainer.h"
#include "textureshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool TextureShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"./texture.vs", L"./texture.ps");
  if (!result) {
    return false;
  }

  return true;
}

void TextureShaderClass::Shutdown() { ShutdownShader(); }

bool TextureShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto baseViewMatrix = parameters.GetMatrix("baseViewMatrix");
  auto orthoMatrix = parameters.GetMatrix("orthoMatrix");
  auto texture = parameters.GetTexture("texture");

  auto result =
      SetShaderParameters(worldMatrix, baseViewMatrix, orthoMatrix, texture);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool TextureShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                          WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
  unsigned int numElements;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_SAMPLER_DESC samplerDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(vsFilename, NULL, NULL, "TextureVertexShader",
                              "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(psFilename, NULL, NULL, "TexturePixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }
  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      vertex_shader_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     pixel_shader_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  polygonLayout[1].SemanticName = "TEXCOORD";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  polygonLayout[1].InputSlot = 0;
  polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[1].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), layout_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL,
                                matrix_buffer_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  result =
      device->CreateSamplerState(&samplerDesc, sample_state_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void TextureShaderClass::ShutdownShader() {

  if (sample_state_) {
    sample_state_.Reset();
    sample_state_ = nullptr;
  }

  if (matrix_buffer_) {
    matrix_buffer_.Reset();
    matrix_buffer_ = nullptr;
  }

  if (layout_) {
    layout_.Reset();
    layout_ = nullptr;
  }

  if (pixel_shader_) {
    pixel_shader_.Reset();
    pixel_shader_ = nullptr;
  }

  if (vertex_shader_) {
    vertex_shader_.Reset();
    vertex_shader_ = nullptr;
  }
}

void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                                  HWND hwnd,
                                                  WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool TextureShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  MatrixBufferType *dataPtr;
  unsigned int buffer_number;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &texture);

  return true;
}

void TextureShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

  device_context->DrawIndexed(indexCount, 0, 0);
}

// File: C:\Users\jipengli\OneDrive - Advanced Micro Devices
// Inc\个人文档\DirectX_Study\DirectX_Study\DirectX11_study\DX11 Base
// Tutorials\31_soft_shadow\verticalblurshaderclass.cpp

#include "../CommonFramework/DirectX11Device.h"
#include "verticalblurshaderclass.h"

#include <d3dcompiler.h>
#include <fstream>

using namespace std;
using namespace DirectX;

bool VerticalBlurShaderClass::Initialize(HWND hwnd) {
  bool result;

  result = InitializeShader(hwnd, L"verticalblur.vs", L"verticalblur.ps");
  if (!result) {
    return false;
  }

  return true;
}

void VerticalBlurShaderClass::Shutdown() { ShutdownShader(); }

bool VerticalBlurShaderClass::Render(
    int indexCount, const ShaderParameterContainer &parameters) const {

  auto worldMatrix = parameters.GetMatrix("worldMatrix");
  auto viewMatrix = parameters.GetMatrix("viewMatrix");
  auto projectionMatrix = parameters.GetMatrix("projectionMatrix");
  auto screenHeight = parameters.GetFloat("screenHeight");
  auto texture = parameters.GetTexture("texture");

  auto result = SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix,
                                    texture, screenHeight);
  if (!result) {
    return false;
  }

  RenderShader(indexCount);

  return true;
}

bool VerticalBlurShaderClass::InitializeShader(HWND hwnd, WCHAR *vsFilename,
                                               WCHAR *psFilename) {
  HRESULT result;
  ID3D10Blob *errorMessage;
  ID3D10Blob *vertexShaderBuffer;
  ID3D10Blob *pixelShaderBuffer;
  D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
  unsigned int numElements;
  D3D11_SAMPLER_DESC samplerDesc;
  D3D11_BUFFER_DESC matrixBufferDesc;
  D3D11_BUFFER_DESC screenSizeBufferDesc;

  errorMessage = 0;
  vertexShaderBuffer = 0;
  pixelShaderBuffer = 0;

  result = D3DCompileFromFile(
      vsFilename, NULL, NULL, "VerticalBlurVertexShader", "vs_5_0",
      D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
    }

    else {
      MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  result = D3DCompileFromFile(psFilename, NULL, NULL, "VerticalBlurPixelShader",
                              "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
                              &pixelShaderBuffer, &errorMessage);
  if (FAILED(result)) {

    if (errorMessage) {
      OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
    }

    else {
      MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
    }

    return false;
  }

  auto device = DirectX11Device::GetD3d11DeviceInstance()->GetDevice();

  result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
                                      vertexShaderBuffer->GetBufferSize(), NULL,
                                      vertex_shader_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
                                     pixelShaderBuffer->GetBufferSize(), NULL,
                                     pixel_shader_.GetAddressOf());
  if (FAILED(result)) {
    return false;
  }

  polygonLayout[0].SemanticName = "POSITION";
  polygonLayout[0].SemanticIndex = 0;
  polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygonLayout[0].InputSlot = 0;
  polygonLayout[0].AlignedByteOffset = 0;
  polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[0].InstanceDataStepRate = 0;

  polygonLayout[1].SemanticName = "TEXCOORD";
  polygonLayout[1].SemanticIndex = 0;
  polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  polygonLayout[1].InputSlot = 0;
  polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygonLayout[1].InstanceDataStepRate = 0;

  numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

  result = device->CreateInputLayout(
      polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(), &layout_);
  if (FAILED(result)) {
    return false;
  }

  vertexShaderBuffer->Release();
  vertexShaderBuffer = 0;

  pixelShaderBuffer->Release();
  pixelShaderBuffer = 0;

  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.MipLODBias = 0.0f;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  samplerDesc.BorderColor[0] = 0;
  samplerDesc.BorderColor[1] = 0;
  samplerDesc.BorderColor[2] = 0;
  samplerDesc.BorderColor[3] = 0;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  result = device->CreateSamplerState(&samplerDesc, &sample_state_);
  if (FAILED(result)) {
    return false;
  }

  matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
  matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrixBufferDesc.MiscFlags = 0;
  matrixBufferDesc.StructureByteStride = 0;

  result = device->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);
  if (FAILED(result)) {
    return false;
  }

  screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBufferType);
  screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  screenSizeBufferDesc.MiscFlags = 0;
  screenSizeBufferDesc.StructureByteStride = 0;

  result =
      device->CreateBuffer(&screenSizeBufferDesc, NULL, &screen_size_buffer_);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void VerticalBlurShaderClass::ShutdownShader() {}

void VerticalBlurShaderClass::OutputShaderErrorMessage(ID3D10Blob *errorMessage,
                                                       HWND hwnd,
                                                       WCHAR *shaderFilename) {
  char *compileErrors;
  SIZE_T bufferSize, i;
  ofstream fout;

  compileErrors = (char *)(errorMessage->GetBufferPointer());

  bufferSize = errorMessage->GetBufferSize();

  fout.open("shader-error.txt");

  for (i = 0; i < bufferSize; i++) {
    fout << compileErrors[i];
  }

  fout.close();

  errorMessage->Release();
  errorMessage = 0;

  MessageBox(hwnd,
             L"Error compiling shader.  Check shader-error.txt for message.",
             shaderFilename, MB_OK);
}

bool VerticalBlurShaderClass::SetShaderParameters(
    const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix,
    const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView *texture,
    float screenHeight) const {
  HRESULT result;
  D3D11_MAPPED_SUBRESOURCE mappedResource;
  MatrixBufferType *dataPtr;
  unsigned int buffer_number;
  ScreenSizeBufferType *dataPtr2;

  XMMATRIX worldMatrixCopy = worldMatrix;
  XMMATRIX viewMatrixCopy = viewMatrix;
  XMMATRIX projectionMatrixCopy = projectionMatrix;

  worldMatrixCopy = XMMatrixTranspose(worldMatrix);
  viewMatrixCopy = XMMatrixTranspose(viewMatrix);
  projectionMatrixCopy = XMMatrixTranspose(projectionMatrix);

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  result = device_context->Map(matrix_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr = (MatrixBufferType *)mappedResource.pData;

  dataPtr->world = worldMatrixCopy;
  dataPtr->view = viewMatrixCopy;
  dataPtr->projection = projectionMatrixCopy;

  device_context->Unmap(matrix_buffer_.Get(), 0);

  buffer_number = 0;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       matrix_buffer_.GetAddressOf());

  result = device_context->Map(screen_size_buffer_.Get(), 0,
                               D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
  if (FAILED(result)) {
    return false;
  }

  dataPtr2 = (ScreenSizeBufferType *)mappedResource.pData;

  dataPtr2->screenHeight = screenHeight;
  dataPtr2->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);

  device_context->Unmap(screen_size_buffer_.Get(), 0);

  buffer_number = 1;

  device_context->VSSetConstantBuffers(buffer_number, 1,
                                       screen_size_buffer_.GetAddressOf());

  device_context->PSSetShaderResources(0, 1, &texture);

  return true;
}

void VerticalBlurShaderClass::RenderShader(int indexCount) const {

  auto device_context =
      DirectX11Device::GetD3d11DeviceInstance()->GetDeviceContext();

  device_context->IASetInputLayout(layout_.Get());

  device_context->VSSetShader(vertex_shader_.Get(), NULL, 0);
  device_context->PSSetShader(pixel_shader_.Get(), NULL, 0);

  device_context->PSSetSamplers(0, 1, sample_state_.GetAddressOf());

  device_context->DrawIndexed(indexCount, 0, 0);
}