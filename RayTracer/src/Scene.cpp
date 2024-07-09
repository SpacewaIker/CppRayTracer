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
std::optional<Light> ParseLight(const toml::table *table);

std::unique_ptr<Geometry> ParseGeometry(const toml::table *table);
std::unique_ptr<SDFGeometry> ParseSDFGeometry(const toml::table *table);

std::unique_ptr<Sphere> ParseSphere(const toml::table *table);
std::unique_ptr<Plane> ParsePlane(const toml::table *table);
std::unique_ptr<AABB> ParseAABB(const toml::table *table);
std::unique_ptr<Transform> ParseTransform(const toml::table *table);
std::unique_ptr<SDFSphere> ParseSDFSphere(const toml::table *table);
std::unique_ptr<SDFPlane> ParseSDFPlane(const toml::table *table);
std::unique_ptr<SDFAABB> ParseSDFAABB(const toml::table *table);
std::unique_ptr<SDFHollowSphere> ParseSDFHollowSphere(const toml::table *table);
std::unique_ptr<SDFConstructive> ParseSDFConstructive(const toml::table *table);

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

std::unique_ptr<Geometry> ParseGeometry(const toml::table *table) {
    std::string type = table->get_as<std::string>("type")->value_or("");
    std::transform(type.begin(), type.end(), type.begin(), ::tolower);

    if (type == "sphere") {
        return ParseSphere(table);
    } else if (type == "plane") {
        return ParsePlane(table);
    } else if (type == "aabb") {
        return ParseAABB(table);
    } else if (type == "transform") {
        return ParseTransform(table);
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
        return ParseSDFSphere(table);
    } else if (type == "sdfplane") {
        return ParseSDFPlane(table);
    } else if (type == "sdfaabb") {
        return ParseSDFAABB(table);
    } else if (type == "sdfhollowsphere") {
        return ParseSDFHollowSphere(table);
    } else if (type == "sdfconstructive") {
        return ParseSDFConstructive(table);
    } else {
        std::cerr << "Unknown SDF geometry type: " << type << ". skipping..." << std::endl;
    }

    return nullptr;
}

std::unique_ptr<Sphere> ParseSphere(const toml::table *table) {
    auto position = table->get_as<toml::array>("position");
    auto radius = table->get_as<toml::value<double>>("radius");
    auto material = table->get_as<toml::value<int64_t>>("material");
    int materialValid = material ? (int)material->get() : 0;

    if (position && radius) {
        return std::make_unique<Sphere>(ParseVec3(position), (float)radius->get(), materialValid);
    }

    std::cerr << "Invalid sphere geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<Plane> ParsePlane(const toml::table *table) {
    auto position = table->get_as<toml::array>("position");
    auto normal = table->get_as<toml::array>("normal");
    auto material = table->get_as<toml::value<int64_t>>("material");
    auto material2 = table->get_as<toml::value<int64_t>>("material2");
    int materialValid = material ? (int)material->get() : 0;
    int material2Valid = material2 ? (int)material2->get() : -1;

    if (position && normal) {
        return std::make_unique<Plane>(ParseVec3(position), ParseVec3(normal), materialValid,
                                       material2Valid);
    }

    std::cerr << "Invalid plane geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<AABB> ParseAABB(const toml::table *table) {
    auto min = table->get_as<toml::array>("min");
    auto max = table->get_as<toml::array>("max");
    auto material = table->get_as<toml::value<int64_t>>("material");
    int materialValid = material ? (int)material->get() : 0;

    if (min && max) {
        return std::make_unique<AABB>(ParseVec3(min), ParseVec3(max), materialValid);
    }

    std::cerr << "Invalid AABB geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<Transform> ParseTransform(const toml::table *table) {
    auto translation = table->get_as<toml::array>("translation");
    glm::vec3 translationValid = translation ? ParseVec3(translation) : glm::vec3{0.0f};
    auto rotation = table->get_as<toml::array>("rotation");
    glm::vec3 rotationValid = rotation ? ParseVec3(rotation) : glm::vec3{0.0f};
    auto scale = table->get_as<toml::array>("scale");
    glm::vec3 scaleValid = scale ? ParseVec3(scale) : glm::vec3{1.0f};
    auto child = table->get_as<toml::table>("child");

    if (child) {
        auto childParsed = ParseGeometry(child);
        if (childParsed) {
            return std::make_unique<Transform>(translationValid, rotationValid, scaleValid,
                                               std::move(childParsed));
        }
    }

    std::cerr << "Invalid transform geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<SDFSphere> ParseSDFSphere(const toml::table *table) {
    auto position = table->get_as<toml::array>("position");
    auto radius = table->get_as<toml::value<double>>("radius");
    auto material = table->get_as<toml::value<int64_t>>("material");
    int materialValid = material ? (int)material->get() : 0;

    if (position && radius) {
        return std::make_unique<SDFSphere>(ParseVec3(position), (float)radius->get(),
                                           materialValid);
    }

    std::cerr << "Invalid sdfsphere geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<SDFPlane> ParseSDFPlane(const toml::table *table) {
    auto position = table->get_as<toml::array>("position");
    auto normal = table->get_as<toml::array>("normal");
    auto material = table->get_as<toml::value<int64_t>>("material");
    auto material2 = table->get_as<toml::value<int64_t>>("material2");
    int materialValid = material ? (int)material->get() : 0;
    int material2Valid = material2 ? (int)material2->get() : -1;

    if (position && normal) {
        return std::make_unique<SDFPlane>(ParseVec3(position), ParseVec3(normal), materialValid,
                                          material2Valid);
    }

    std::cerr << "Invalid sdfplane geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<SDFAABB> ParseSDFAABB(const toml::table *table) {
    auto min = table->get_as<toml::array>("min");
    auto max = table->get_as<toml::array>("max");
    auto material = table->get_as<toml::value<int64_t>>("material");
    int materialValid = material ? (int)material->get() : 0;
    auto rounded = table->get_as<toml::value<double>>("rounded");
    float roundedValid = rounded ? (float)rounded->get() : 0.0f;

    if (min && max) {
        return std::make_unique<SDFAABB>(ParseVec3(min), ParseVec3(max), roundedValid,
                                         materialValid);
    }

    std::cerr << "Invalid sdfAABB geometry. skipping..." << std::endl;
    return nullptr;
}

std::unique_ptr<SDFHollowSphere> ParseSDFHollowSphere(const toml::table *table) {
    auto position = table->get_as<toml::array>("position");
    auto radius = table->get_as<toml::value<double>>("radius");
    auto thickness = table->get_as<toml::value<double>>("thickness");
    auto height = table->get_as<toml::value<double>>("height");
    auto material = table->get_as<toml::value<int64_t>>("material");
    int materialValid = material ? (int)material->get() : 0;

    if (position && radius && thickness && height) {
        return std::make_unique<SDFHollowSphere>(ParseVec3(position), (float)radius->get(),
                                                 (float)thickness->get(), (float)height->get(),
                                                 materialValid);
    }
    return nullptr;
}

std::unique_ptr<SDFConstructive> ParseSDFConstructive(const toml::table *table) {
    std::string operation = table->get_as<std::string>("operation")->value_or("");
    auto material = table->get_as<toml::value<int64_t>>("material");
    int materialValid = material ? (int)material->get() : 0;
    auto smoothing = table->get_as<toml::value<double>>("smoothing");
    float smoothingValid = smoothing ? (float)smoothing->get() : 1.0f;

    SDFConstructive::Operation op;
    if (operation == "union") {
        op =
            smoothing ? SDFConstructive::Operation::SmoothUnion : SDFConstructive::Operation::Union;
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

    if (left && right) {
        auto leftParsed = ParseSDFGeometry(left);
        auto rightParsed = ParseSDFGeometry(right);

        if (leftParsed && rightParsed) {
            return std::make_unique<SDFConstructive>(
                op, std::move(leftParsed), std::move(rightParsed), materialValid, smoothingValid);
        }
    }

    std::cerr << "Invalid constructive SDF geometry. skipping..." << std::endl;
    return nullptr;
}

Scene SceneLoader::LoadScene(const std::string &path) {
    toml::table table;

    // load scene
    try {
        table = toml::parse_file("scene.toml");
    } catch (const toml::parse_error &err) {
        std::cerr << "Failed to parse scene.toml: " << err << std::endl;
        exit(1);
    }

    Scene scene;

    // materials
    if (table["materials"].is_array()) {
        for (const auto &material : *table["materials"].as_array()) {
            scene.Materials.push_back(ParseMaterial(material.as_table()));
        }
    } else {
        std::cerr << "No materials found. At least one material is required." << std::endl;
        exit(1);
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
