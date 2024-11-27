#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <vector>

typedef ID3D11RasterizerState *RasterizerStatePtr;
typedef D3D11_RASTERIZER_DESC RasterizerDescription;
typedef D3D11_FILL_MODE FillMode;
typedef D3D11_CULL_MODE CullMode;
typedef std::vector<ID3D11RasterizerState *> RasterizerStateVector;
typedef std::unordered_map<std::string, unsigned int> RasterizerNameMap;

class RasterizationStateManager {
public:
  RasterizationStateManager();

  RasterizationStateManager(const RasterizationStateManager &rhs) = delete;
  ;

  RasterizationStateManager &
  operator=(const RasterizationStateManager &rhs) = delete;

  ~RasterizationStateManager();

public:
  unsigned int CreateNewRaterizerStateNoDesc();

  unsigned int CreateNewRaterizerState(D3D11_RASTERIZER_DESC &desc);

  void PushNewRasterizerStateWithName(std::string name, unsigned int index);

  RasterizerStatePtr GetRasterizerStateByName(std::string name);

  void SetCurrentRasterizerStateByName(std::string name);

public:
  void ChangeCurrentFillMode(D3D11_FILL_MODE fillMode);

  void ChangeCurrnetCullMode(D3D11_CULL_MODE cullMode);

  void SetCurrnetFrontCounterClockwise(bool trueOrFalse);

  void SetCurrentDepthBias(INT bias);

  void SetCurrentDepthBiasClamp(FLOAT clamp);

  void SetCurrentSlopeScaledDepthBias(FLOAT bias);

  void SetCurrentDepthClipEnable(bool trueOrFalse);

  void SetCurrentScissorEnable(bool trueOrFalse);

  void SetCurrentMultisampleEnable(bool trueOfFalse);

  void SetCurrentAntialiasedLineEnable(bool trueOrFalse);

private:
  RasterizerStateVector rasterizer_state_vector_{};

  RasterizerStateVector retired_state_vector_{};

  RasterizerNameMap rasterizer_name_map_{};
};