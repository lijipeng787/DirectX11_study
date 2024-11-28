#pragma once

#include <DirectXMath.h>

struct ModelInfoType;

class ModelListClass {
public:
  ModelListClass() {}

  ModelListClass(const ModelListClass &rhs) = delete;

  ~ModelListClass() {}

public:
  bool Initialize(int);

  void Shutdown();

  int GetModelCount();

  void GetData(int, float &, float &, float &, DirectX::XMFLOAT4 &);

private:
  int model_count_ = 0;

  ModelInfoType *model_info_list_ = nullptr;
};