#include "ShaderParameterContainer.h"

// Strongly typed ShaderParameterContainer is now implemented inline in the
// header to keep variant-based helpers available to all translation units.

bool ShaderParameterContainer::type_mismatch_logging_enabled_ = true;

void ShaderParameterContainer::SetTypeMismatchLoggingEnabled(bool enabled) {
	type_mismatch_logging_enabled_ = enabled;
}