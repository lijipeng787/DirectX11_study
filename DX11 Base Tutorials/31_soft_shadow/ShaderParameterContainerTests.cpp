#include "ShaderParameterContainerTests.h"

#include "Logger.h"
#include "ShaderParameterContainer.h"

#include <DirectXMath.h>

#include <cmath>
#include <exception>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct TestCaseResult {
  std::string name;
  bool passed = false;
  std::string message;
};

constexpr float kFloatTolerance = 1e-5f;

bool AreFloatsEqual(float lhs, float rhs) {
  return std::fabs(lhs - rhs) <= kFloatTolerance;
}

bool TestMergeWithPriorityOverrides() {
  ShaderParameterContainer global_parameters;
  ShaderParameterContainer pass_parameters;

  global_parameters.SetFloat("shadowStrength", 0.2f);
  pass_parameters.SetFloat("shadowStrength", 0.55f);

  DirectX::XMFLOAT4 ambient_global{0.1f, 0.1f, 0.1f, 1.0f};
  DirectX::XMFLOAT4 ambient_pass{0.3f, 0.3f, 0.3f, 1.0f};
  global_parameters.SetVector4("ambientColor", ambient_global);
  pass_parameters.SetVector4("ambientColor", ambient_pass);

  auto merged = ShaderParameterContainer::MergeWithPriority(global_parameters,
                                                            pass_parameters);

  if (!AreFloatsEqual(merged.GetFloat("shadowStrength"), 0.55f)) {
    return false;
  }

  const auto merged_ambient = merged.GetVector4("ambientColor");
  return AreFloatsEqual(merged_ambient.x, ambient_pass.x) &&
         AreFloatsEqual(merged_ambient.y, ambient_pass.y) &&
         AreFloatsEqual(merged_ambient.z, ambient_pass.z) &&
         AreFloatsEqual(merged_ambient.w, ambient_pass.w);
}

bool TestChainMergePriorityOrder() {
  ShaderParameterContainer global_parameters;
  ShaderParameterContainer pass_parameters;
  ShaderParameterContainer object_parameters;
  ShaderParameterContainer callback_parameters;

  global_parameters.SetFloat("shadowStrength", 0.1f);
  pass_parameters.SetFloat("shadowStrength", 0.4f);
  object_parameters.SetFloat("shadowStrength", 0.7f);
  callback_parameters.SetFloat("shadowStrength", 1.0f);

  auto full_chain = ShaderParameterContainer::ChainMerge(
      global_parameters, pass_parameters, &object_parameters,
      &callback_parameters);
  auto no_callback_chain = ShaderParameterContainer::ChainMerge(
      global_parameters, pass_parameters, &object_parameters, nullptr);
  auto object_only_chain = ShaderParameterContainer::ChainMerge(
      global_parameters, pass_parameters, nullptr, nullptr);

  return AreFloatsEqual(full_chain.GetFloat("shadowStrength"), 1.0f) &&
         AreFloatsEqual(no_callback_chain.GetFloat("shadowStrength"), 0.7f) &&
         AreFloatsEqual(object_only_chain.GetFloat("shadowStrength"), 0.4f);
}

bool TestTypeConflictThrows() {
  struct ScopedLoggingGuard {
    ScopedLoggingGuard() {
      ShaderParameterContainer::SetTypeMismatchLoggingEnabled(false);
    }
    ~ScopedLoggingGuard() {
      ShaderParameterContainer::SetTypeMismatchLoggingEnabled(true);
    }
  } guard;

  ShaderParameterContainer global_parameters;
  ShaderParameterContainer pass_parameters;

  global_parameters.SetFloat("worldMatrix", 1.0F);
  pass_parameters.SetMatrix("worldMatrix", DirectX::XMMatrixIdentity());

  try {
    (void)ShaderParameterContainer::MergeWithPriority(global_parameters,
                                                      pass_parameters);
  } catch (const std::runtime_error &) {
    return true;
  } catch (...) {
    return false;
  }
  return false;
}

std::vector<TestCaseResult> RunAllTestsInternal() {
  std::vector<TestCaseResult> results;
  results.reserve(3);

  auto run = [&results](const std::string &name, auto &&callable) {
    TestCaseResult result{name};
    try {
      result.passed = callable();
      if (!result.passed) {
        result.message = "Assertion failed";
      }
    } catch (const std::exception &ex) {
      result.passed = false;
      result.message = ex.what();
    } catch (...) {
      result.passed = false;
      result.message = "Unknown exception";
    }
    results.push_back(result);
  };

  run("MergeWithPriority overrides higher precedence",
      [] { return TestMergeWithPriorityOverrides(); });
  run("ChainMerge respects priority order",
      [] { return TestChainMergePriorityOrder(); });
  run("MergeWithPriority detects type conflicts",
      [] { return TestTypeConflictThrows(); });

  return results;
}

} // namespace

bool RunShaderParameterContainerTests() {
  const auto results = RunAllTestsInternal();
  bool all_passed = true;

  for (const auto &result : results) {
    if (!result.passed) {
      all_passed = false;
      std::ostringstream oss;
      oss << "Test failed: " << result.name;
      if (!result.message.empty()) {
        oss << " - " << result.message;
      }
      Logger::SetModule("ShaderParameterContainerTests");
      Logger::LogError(oss.str());
    }
  }

  if (all_passed) {
    Logger::SetModule("ShaderParameterContainerTests");
    Logger::LogInfo("All ShaderParameterContainer tests passed");
  }

  return all_passed;
}
