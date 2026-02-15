#pragma once

#include <memory>
#include <unordered_map>

#include "enums/ShaderObjectType.h"
#include "materials/ShaderObject.h"

typedef std::unordered_map<ShaderObjectType, std::shared_ptr<material::Material>> SubpassShaderList;
