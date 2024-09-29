#pragma once

#include <functional>

#include "ShaderParameterContainer.h"

using ShaderParameterCallback = std::function<void(ShaderParameterContainer &)>;