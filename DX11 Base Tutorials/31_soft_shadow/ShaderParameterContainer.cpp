#include "ShaderParameter.h"

// Legacy translation unit maintained for build compatibility.

void ShaderParameterContainer::SetTypeMismatchLoggingEnabled(bool enabled) {
  type_mismatch_logging_enabled_ = enabled;
}

void ShaderParameterContainer::SetOverrideLoggingEnabled(bool enabled) {
  override_logging_enabled_ = enabled;
}