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
    if (auto emissionColour = table->get_as<toml::array>("emission_colour")) {
        mat.EmissionColour = ParseVec3(emissionColour);
    }
    if (auto emissionPower = table->get_as<toml::value<double>>("emission_power")) {
        mat.EmissionPower = (float)emissionPower->get();
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
    } else if (type == "sdfsphere" || type == "sdfhollowsphere" || type == "sdfplane" || type == "sdfconstructive" ||
               type == "sdfaabb") {
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
        return std::make_unique<Plane>(ParseVec3(position), ParseVec3(normal), materialValid, material2Valid);
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
            return std::make_unique<Transform>(translationValid, rotationValid, scaleValid, std::move(childParsed));
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
        return std::make_unique<SDFSphere>(ParseVec3(position), (float)radius->get(), materialValid);
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
        return std::make_unique<SDFPlane>(ParseVec3(position), ParseVec3(normal), materialValid, material2Valid);
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
        return std::make_unique<SDFAABB>(ParseVec3(min), ParseVec3(max), roundedValid, materialValid);
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
        return std::make_unique<SDFHollowSphere>(ParseVec3(position), (float)radius->get(), (float)thickness->get(),
                                                 (float)height->get(), materialValid);
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
        op = smoothing ? SDFConstructive::Operation::SmoothUnion : SDFConstructive::Operation::Union;
    } else if (operation == "difference") {
        op = smoothing ? SDFConstructive::Operation::SmoothDifference : SDFConstructive::Operation::Difference;
    } else if (operation == "intersection") {
        op = smoothing ? SDFConstructive::Operation::SmoothIntersection : SDFConstructive::Operation::Intersection;
    } else {
        std::cerr << "Invalid operation for constructive SDF: " << operation << ". skipping..." << std::endl;
        return nullptr;
    }

    auto left = table->get_as<toml::table>("left");
    auto right = table->get_as<toml::table>("right");

    if (left && right) {
        auto leftParsed = ParseSDFGeometry(left);
        auto rightParsed = ParseSDFGeometry(right);

        if (leftParsed && rightParsed) {
            return std::make_unique<SDFConstructive>(op, std::move(leftParsed), std::move(rightParsed), materialValid,
                                                     smoothingValid);
        }
    }

    std::cerr << "Invalid constructive SDF geometry. skipping..." << std::endl;
    return nullptr;
}

Scene SceneLoader::LoadScene(const std::string &path) {
    toml::table table;

    // load scene file
    try {
        table = toml::parse_file("scene.toml");
    } catch (const toml::parse_error &err) {
        std::cerr << "Failed to parse scene.toml: " << err << std::endl;
        exit(1);
    }

    Scene scene;

    if (auto skyColour = table.get_as<toml::array>("sky_colour")) {
        scene.SkyColour = ParseVec3(skyColour);
    }

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

Camera SceneLoader::LoadCameraSettings(const std::string &path) {
    toml::table table;

    // load scene file
    try {
        table = toml::parse_file("scene.toml");
    } catch (const toml::parse_error &err) {
        std::cerr << "Failed to parse scene.toml: " << err << std::endl;
        exit(1);
    }

    auto verticalFOV = table.get_as<toml::value<double>>("vertical_fov");
    float verticalFOVValid = verticalFOV ? (float)verticalFOV->get() : 45.0f;
    auto nearPlane = table.get_as<toml::value<double>>("near_plane");
    float nearPlaneValid = nearPlane ? (float)nearPlane->get() : 0.1f;
    auto farPlane = table.get_as<toml::value<double>>("far_plane");
    float farPlaneValid = farPlane ? (float)farPlane->get() : 100.0f;
    auto position = table.get_as<toml::array>("camera_position");
    auto positionValid = position ? ParseVec3(position) : glm::vec3{0.0f};
    auto forwardDirection = table.get_as<toml::array>("camera_forward_direction");
    glm::vec3 forwardDirectionValid = forwardDirection ? ParseVec3(forwardDirection) : glm::vec3{0.0f, 0.0f, 1.0f};

    return Camera(verticalFOVValid, nearPlaneValid, farPlaneValid, positionValid, forwardDirectionValid);
}

void SceneLoader::SaveScene(const std::string &path, const Scene &scene, const Camera &camera) {
    toml::table table;

    // sky colour
    table.insert("sky_colour", toml::array{scene.SkyColour.x, scene.SkyColour.y, scene.SkyColour.z});

    // camera settings
    table.insert("vertical_fov", camera.GetSettings().VerticalFOV);
    table.insert("near_plane", camera.GetSettings().NearPlane);
    table.insert("far_plane", camera.GetSettings().FarPlane);
    glm::vec3 position = camera.GetSettings().Position;
    table.insert("camera_position", toml::array{position.x, position.y, position.z});
    glm::vec3 forwardDirection = camera.GetSettings().ForwardDirection;
    table.insert("camera_forward_direction", toml::array{forwardDirection.x, forwardDirection.y, forwardDirection.z});

    // materials
    // toml::array materials;
    // for (const auto &material : scene.Materials) {
    //     toml::table materialTable;
    //     materialTable["colour"] = toml::array{material.Colour.x, material.Colour.y, material.Colour.z};
    //     materialTable["emission"] = toml::array{material.Emission.x, material.Emission.y, material.Emission.z};
    //     materialTable["reflection"] = material.Reflection;
    //     materialTable["refraction"] = material.Refraction;
    //     materialTable["ior"] = material.IOR;
    //     materials.push_back(materialTable);
    // }
    // table["materials"] = materials;
    //
    // // geometry
    // toml::array geometry;
    // for (const auto &g : scene.Geometry) {
    //     toml::table geometryTable;
    //     g->Save(geometryTable);
    //     geometry.push_back(geometryTable);
    // }
    // table["geometry"] = geometry;
    //
    // // lights
    // toml::array lights;
    // for (const auto &light : scene.Lights) {
    //     toml::table lightTable;
    //     lightTable["position"] = toml::array{light.Position.x, light.Position.y, light.Position.z};
    //     lightTable["colour"] = toml::array{light.Colour.x, light.Colour.y, light.Colour.z};
    //     lightTable["intensity"] = light.Intensity;
    //     lights.push_back(lightTable);
    // }
    // table["lights"] = lights;

    // save scene file
    std::ofstream file(path);
    file << table;
    file.close();
}
