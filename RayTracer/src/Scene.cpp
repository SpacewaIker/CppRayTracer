#include "Scene.h"

#include "Geometry/AABB.h"
#include "Geometry/Plane.h"
#include "Geometry/SDF/SDFAABB.h"
#include "Geometry/SDF/SDFConstructive.h"
#include "Geometry/SDF/SDFHollowSphere.h"
#include "Geometry/SDF/SDFPlane.h"
#include "Geometry/SDF/SDFSphere.h"
#include "Geometry/Sphere.h"
#include "Geometry/Transform.h"
#include "Light.h"
#include "toml++/toml.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

// prototypes
glm::vec3 ParseVec3(const toml::array *array);
Material ParseMaterial(const toml::table *table);
std::unique_ptr<Geometry> ParseGeometry(const toml::table *table);
std::unique_ptr<SDFGeometry> ParseSDFGeometry(const toml::table *table);
std::optional<Light> ParseLight(const toml::table *table);

// implementations
glm::vec3 ParseVec3(const toml::array *array) {
    glm::vec3 vec{0.0f};

    vec.x = array->at(0).value_or(0.0f);
    vec.y = array->at(1).value_or(0.0f);
    vec.z = array->at(2).value_or(0.0f);

    return vec;
}

Material ParseMaterial(const toml::table *table) {
    Material mat;

    if (auto albedo = table->get_as<toml::array>("albedo")) {
        mat.Albedo = ParseVec3(albedo);
    }
    if (auto roughness = table->get_as<toml::value<double>>("roughness")) {
        mat.Roughness = (float)roughness->get();
    }
    if (auto metallic = table->get_as<toml::value<double>>("metallic")) {
        mat.Metallic = (float)metallic->get();
    }

    return mat;
}

std::unique_ptr<Geometry> ParseGeometry(const toml::table *table) {
    std::string type = table->get_as<std::string>("type")->value_or("");
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    if (type == "sphere") {
        auto position = table->get_as<toml::array>("position");
        auto radius = table->get_as<toml::value<double>>("radius");
        auto material = table->get_as<toml::value<int64_t>>("material");

        if (position && radius && material) {
            return std::make_unique<Sphere>(ParseVec3(position), (float)radius->get(),
                                            (int)material->get());
        }

        std::cerr << "Invalid sphere geometry. skipping..." << std::endl;
    } else if (type == "plane") {
        auto position = table->get_as<toml::array>("position");
        auto normal = table->get_as<toml::array>("normal");
        auto material = table->get_as<toml::value<int64_t>>("material");
        auto material2 = table->get_as<toml::value<int64_t>>("material2");

        if (position && normal && material && material2) {
            return std::make_unique<Plane>(ParseVec3(position), ParseVec3(normal),
                                           (int)material->get(), (int)material2->get());
        }

        std::cerr << "Invalid plane geometry. skipping..." << std::endl;
    } else if (type == "aabb") {
        auto min = table->get_as<toml::array>("min");
        auto max = table->get_as<toml::array>("max");
        auto material = table->get_as<toml::value<int64_t>>("material");

        if (min && max && material) {
            return std::make_unique<AABB>(ParseVec3(min), ParseVec3(max), (int)material->get());
        }

        std::cerr << "Invalid AABB geometry. skipping..." << std::endl;
    } else if (type == "transform") {
        auto translation = table->get_as<toml::array>("translation");
        glm::vec3 translationValid{0.0f};
        if (translation) {
            translationValid = ParseVec3(translation);
        }

        auto rotation = table->get_as<toml::array>("rotation");
        glm::vec3 rotationValid{0.0f};
        if (rotation) {
            rotationValid = ParseVec3(rotation);
        }

        auto scale = table->get_as<toml::array>("scale");
        glm::vec3 scaleValid{1.0f};
        if (scale) {
            scaleValid = ParseVec3(scale);
        }

        auto child = table->get_as<toml::table>("child");

        if (child) {
            auto childParsed = ParseGeometry(child);
            if (childParsed) {
                return std::make_unique<Transform>(translationValid, rotationValid, scaleValid,
                                                   std::move(childParsed));
            }
        }

        std::cerr << "Invalid transform geometry. skipping..." << std::endl;
    } else if (type == "sdfsphere" || type == "sdfhollowsphere" || type == "sdfplane" ||
               type == "sdfconstructive" || type == "sdfaabb") {
        return ParseSDFGeometry(table);
    } else {
        std::cerr << "Unknown geometry type: " << type << ". skipping..." << std::endl;
    }

    return nullptr;
}

std::unique_ptr<SDFGeometry> ParseSDFGeometry(const toml::table *table) {
    std::string type = table->get_as<std::string>("type")->value_or("");
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    if (type == "sdfsphere") {
        auto position = table->get_as<toml::array>("position");
        auto radius = table->get_as<toml::value<double>>("radius");
        auto material = table->get_as<toml::value<int64_t>>("material");
        int materialValid = material ? (int)material->get() : 0;

        if (position && radius) {
            return std::make_unique<SDFSphere>(ParseVec3(position), (float)radius->get(),
                                               materialValid);
        }

        std::cerr << "Invalid sdfsphere geometry. skipping..." << std::endl;
    } else if (type == "sdfplane") {
        auto position = table->get_as<toml::array>("position");
        auto normal = table->get_as<toml::array>("normal");
        auto material = table->get_as<toml::value<int64_t>>("material");
        auto material2 = table->get_as<toml::value<int64_t>>("material2");

        if (position && normal && material && material2) {
            return std::make_unique<SDFPlane>(ParseVec3(position), ParseVec3(normal),
                                              (int)material->get(), (int)material2->get());
        }

        std::cerr << "Invalid sdfplane geometry. skipping..." << std::endl;
    } else if (type == "sdfaabb") {
        auto min = table->get_as<toml::array>("min");
        auto max = table->get_as<toml::array>("max");
        auto material = table->get_as<toml::value<int64_t>>("material");
        auto rounded = table->get_as<toml::value<double>>("rounded");
        float roundedValid = 0.0f;
        if (rounded) {
            roundedValid = (float)rounded->get();
        }

        if (min && max && material) {
            return std::make_unique<SDFAABB>(ParseVec3(min), ParseVec3(max), roundedValid,
                                             (int)material->get());
        }

        std::cerr << "Invalid sdfAABB geometry. skipping..." << std::endl;
    } else if (type == "sdfhollowsphere") {
        auto position = table->get_as<toml::array>("position");
        auto radius = table->get_as<toml::value<double>>("radius");
        auto thickness = table->get_as<toml::value<double>>("thickness");
        auto height = table->get_as<toml::value<double>>("height");
        auto material = table->get_as<toml::value<int64_t>>("material");

        if (position && radius && thickness && height && material) {
            return std::make_unique<SDFHollowSphere>(ParseVec3(position), (float)radius->get(),
                                                     (float)thickness->get(), (float)height->get(),
                                                     (int)material->get());
        }
    } else if (type == "sdfconstructive") {
        std::string operation = table->get_as<std::string>("operation")->value_or("");
        auto material = table->get_as<toml::value<int64_t>>("material");
        auto smoothing = table->get_as<toml::value<double>>("smoothing");

        SDFConstructive::Operation op;
        if (operation == "union") {
            op = smoothing ? SDFConstructive::Operation::SmoothUnion
                           : SDFConstructive::Operation::Union;
        } else if (operation == "difference") {
            op = smoothing ? SDFConstructive::Operation::SmoothDifference
                           : SDFConstructive::Operation::Difference;
        } else if (operation == "intersection") {
            op = smoothing ? SDFConstructive::Operation::SmoothIntersection
                           : SDFConstructive::Operation::Intersection;
        } else {
            std::cerr << "Invalid operation for constructive SDF: " << operation << ". skipping..."
                      << std::endl;
            return nullptr;
        }

        auto left = table->get_as<toml::table>("left");
        auto right = table->get_as<toml::table>("right");

        float smoothingValid = 1.0f;
        if (smoothing) {
            smoothingValid = (float)smoothing->get();
        }
        if (left && right && material) {
            auto leftParsed = ParseSDFGeometry(left);
            auto rightParsed = ParseSDFGeometry(right);

            if (leftParsed && rightParsed) {
                return std::make_unique<SDFConstructive>(op, std::move(leftParsed),
                                                         std::move(rightParsed),
                                                         (int)material->get(), smoothingValid);
            }
        }

        std::cerr << "Invalid constructive SDF geometry. skipping..." << std::endl;
    } else {
        std::cerr << "Unknown SDF geometry type: " << type << ". skipping..." << std::endl;
    }

    return nullptr;
}

std::optional<Light> ParseLight(const toml::table *table) {
    auto lightType = *table->get_as<std::string>("type");

    if (lightType == "directional") {
        Light light;
        light.Type = LightType::Directional;

        auto direction = table->get_as<toml::array>("direction");
        auto intensity = table->get_as<toml::value<double>>("intensity");

        if (direction && intensity) {
            light.Direction = glm::normalize(ParseVec3(direction));
            light.Intensity = (float)intensity->get();

            return light;
        }
    } else if (lightType == "point") {
        Light light;
        light.Type = LightType::Point;

        auto position = table->get_as<toml::array>("position");
        auto intensity = table->get_as<toml::value<double>>("intensity");

        if (position && intensity) {
            light.Position = ParseVec3(position);
            light.Intensity = (float)intensity->get();

            return light;
        }
    } else {
        std::cerr << "Unknown light type: " << lightType << ". skipping..." << std::endl;
    }

    return std::nullopt;
}

Scene SceneLoader::LoadScene(const std::string &path) {
    toml::table table;

    // load scene
    try {
        table = toml::parse_file("scene.toml");
    } catch (const toml::parse_error &err) {
        std::cerr << "Failed to parse scene.toml: " << err << std::endl;
        exit;
    }

    Scene scene;

    // materials
    if (table["materials"].is_array()) {
        for (const auto &material : *table["materials"].as_array()) {
            scene.Materials.push_back(ParseMaterial(material.as_table()));
        }
    }

    // geometry
    if (table["geometry"].is_array()) {
        for (const auto &geometry : *table["geometry"].as_array()) {
            auto g = ParseGeometry(geometry.as_table());
            if (g) {
                scene.Geometry.push_back(g.release());
            }
        }
    }

    // lights
    if (table["lights"].is_array()) {
        for (const auto &light : *table["lights"].as_array()) {
            if (auto l = ParseLight(light.as_table())) {
                scene.Lights.push_back(*l);
            }
        }
    }

    return scene;
}
