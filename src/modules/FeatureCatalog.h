#pragma once

#include "modules/Module.h"

#include <vector>

namespace cloud9 {

struct FeatureDescriptor {
    const char* name;
    const char* description;
    ModuleCategory category;
    SafetyClass safety;
    bool implemented;
};

[[nodiscard]] const std::vector<FeatureDescriptor>& featureCatalog();

} // namespace cloud9
