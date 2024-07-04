#pragma once

#include "Geometry/Geometry.h"
#include "glm/glm.hpp"

#include <vector>

struct Scene {
    std::vector<Geometry *> Shapes;
};
