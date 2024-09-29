#include "ShaderParameterContainer.h"

#include <stdexcept>

void ShaderParameterContainer::Merge(const ShaderParameterContainer &other) {
  for (const auto &[name, value] : other.parameters_) {
    // If the parameter already exists in this container, override it
    // If it doesn't exist, add it
    parameters_[name] = value;
  }
}