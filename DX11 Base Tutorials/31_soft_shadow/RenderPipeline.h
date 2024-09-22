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