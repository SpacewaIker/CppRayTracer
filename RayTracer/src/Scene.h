#pragma once

#include "Geometry/Geometry.h"
#include "Material.h"
#include "glm/glm.hpp"

#include <vector>

struct Scene {
    std::vector<Geometry *> Geometry;
    std::vector<Material> Materials;
};
