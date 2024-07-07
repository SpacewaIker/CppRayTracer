#pragma once

#include "Geometry/Geometry.h"
#include "Light.h"
#include "Material.h"
#include "glm/glm.hpp"

#include <string>
#include <vector>

struct Scene {
    std::vector<Geometry *> Geometry;
    std::vector<Material> Materials;
    std::vector<Light> Lights;
    glm::vec3 SkyColour = {0.5f, 0.7f, 0.9f};
};

namespace SceneLoader {
Scene LoadScene(const std::string &path);
}
