#include "ShaderParameterContainerTests.h"

#include "Logger.h"
#include "ShaderParameterContainer.h"

#include <DirectXMath.h>

#include <cmath>
#include <exception>
#include <sstream>
#include <string>
#include <unordered_set>
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

bool TestBuildFinalParametersCallbackPrecedence() {
  constexpr float kGlobalStrength = 0.1F;
  constexpr float kPassStrength = 0.4F;
  constexpr float kObjectStrength = 0.7F;
  constexpr float kCallbackStrength = 1.0F;
  const DirectX::XMFLOAT3 kTranslation{1.0F, 2.0F, 3.0F};

  ShaderParameterContainer global_parameters;
  ShaderParameterContainer pass_parameters;
  ShaderParameterContainer object_parameters;

  global_parameters.SetFloat("shadowStrength", kGlobalStrength,
                             ShaderParameterContainer::ParameterOrigin::Global);
  pass_parameters.SetFloat("shadowStrength", kPassStrength,
                           ShaderParameterContainer::ParameterOrigin::Pass);
  object_parameters.SetFloat("shadowStrength", kObjectStrength,
                             ShaderParameterContainer::ParameterOrigin::Object);

  ShaderParameterContainer base_params =
      ShaderParameterContainer::MergeWithPriority(
          global_parameters, pass_parameters,
          ShaderParameterContainer::ParameterOrigin::Global,
          ShaderParameterContainer::ParameterOrigin::Pass);

  DirectX::XMMATRIX world_matrix = DirectX::XMMatrixTranslation(
      kTranslation.x, kTranslation.y, kTranslation.z);

  ShaderParameterContainer::BuildParametersInput inputs;
  inputs.base_params = &base_params;
  inputs.object_params = &object_parameters;
  inputs.world_matrix = &world_matrix;

  bool callback_invoked = false;
  inputs.callback = [&](ShaderParameterContainer &params) {
    callback_invoked = true;
    params.SetFloat("shadowStrength", kCallbackStrength,
                    ShaderParameterContainer::ParameterOrigin::Callback);
  };

  ShaderParameterContainer final_params =
      ShaderParameterContainer::BuildFinalParameters(inputs);

  if (!callback_invoked) {
    return false;
  }

  return AreFloatsEqual(final_params.GetFloat("shadowStrength"),
                        kCallbackStrength);
}

bool TestLockedParametersPreventOverrides() {
  constexpr float kLockedStrength = 0.0F;
  constexpr float kObjectStrength = 0.6F;
  constexpr float kCallbackStrength = 1.0F;

  ShaderParameterContainer global_parameters;
  ShaderParameterContainer pass_parameters;
  ShaderParameterContainer object_parameters;

  pass_parameters.SetFloatLocked(
      "shadowStrength", kLockedStrength,
      ShaderParameterContainer::ParameterOrigin::Pass);
  object_parameters.SetFloat("shadowStrength", kObjectStrength,
                             ShaderParameterContainer::ParameterOrigin::Object);

  ShaderParameterContainer base_params =
      ShaderParameterContainer::MergeWithPriority(
          global_parameters, pass_parameters,
          ShaderParameterContainer::ParameterOrigin::Global,
          ShaderParameterContainer::ParameterOrigin::Pass);

  ShaderParameterContainer::BuildParametersInput inputs;
  inputs.base_params = &base_params;
  inputs.object_params = &object_parameters;
  inputs.callback = [&](ShaderParameterContainer &params) {
    params.SetFloat("shadowStrength", kCallbackStrength,
                    ShaderParameterContainer::ParameterOrigin::Callback);
  };

  const auto final_params =
      ShaderParameterContainer::BuildFinalParameters(inputs);

  return AreFloatsEqual(final_params.GetFloat("shadowStrength"),
                        kLockedStrength);
}

std::vector<TestCaseResult> RunAllTestsInternal() {
  std::vector<TestCaseResult> results;
  results.reserve(5);

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
  run("BuildFinalParameters keeps callback precedence",
      [] { return TestBuildFinalParametersCallbackPrecedence(); });
  run("Locked parameters prevent overrides",
      [] { return TestLockedParametersPreventOverrides(); });
  run("Prefixed origin dump shows correct prefixes", [] {
    ShaderParameterContainer c;
    c.SetFloat("a", 1.0f, ShaderParameterContainer::ParameterOrigin::Global);
    c.SetFloat("b", 2.0f, ShaderParameterContainer::ParameterOrigin::Pass);
    c.SetFloat("c", 3.0f, ShaderParameterContainer::ParameterOrigin::Object);
    // Simulate callback by overriding default origin via guard
    {
      auto guard = c.OverrideDefaultOrigin(
          ShaderParameterContainer::ParameterOrigin::Callback);
      c.SetFloat("d", 4.0f);
    }
    c.SetFloatLocked("e", 5.0f,
                     ShaderParameterContainer::ParameterOrigin::Pass);
    // Manual (default) origin
    c.SetFloat("m", 9.0f);

    auto pairs = c.GetPrefixedNamePairs();
    std::unordered_set<std::string> names;
    for (auto &p : pairs) {
      names.insert(p.first);
    }
    bool ok = true;
    auto expect = [&](const std::string &needle) {
      if (names.find(needle) == names.end()) {
        ok = false;
      }
    };
    expect("global_a");
    expect("pass_b");
    expect("object_c");
    expect("cb_d");
    expect("pass_e[locked]");
    // Manual prefix may appear as manual_m or unknown_m depending on earlier
    // origin resolution; treat either acceptable.
    if (names.find("manual_m") == names.end() &&
        names.find("unknown_m") == names.end()) {
      ok = false;
    }
    return ok;
  });
  run("StrictValidation throws on locked override", [] {
    ShaderParameterContainer::SetStrictValidationEnabled(true);
    ShaderParameterContainer base;
    base.SetFloatLocked("lockedParam", 0.0f,
                        ShaderParameterContainer::ParameterOrigin::Pass);
    bool threw = false;
    try {
      base.SetFloat("lockedParam", 1.0f,
                    ShaderParameterContainer::ParameterOrigin::Callback);
    } catch (const std::runtime_error &) {
      threw = true;
    }
    ShaderParameterContainer::SetStrictValidationEnabled(false);
    return threw;
  });

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
