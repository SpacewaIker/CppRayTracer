#pragma once

#include "SDFGeometry.h"

#include <memory>

class SDFConstructive : public SDFGeometry {
  public:
    enum class Operation {
        Union,
        SmoothUnion,
        Intersection,
        SmoothIntersection,
        Difference,
        SmoothDifference
    };

    SDFConstructive(Operation operation, std::shared_ptr<SDFGeometry> left,
                    std::shared_ptr<SDFGeometry> right, int materialIndex, float smoothing = 1.0f)
        : m_Operation(operation), m_Left(std::move(left)), m_Right(std::move(right)),
          m_Smoothing(smoothing), SDFGeometry(materialIndex) {}

    float Distance(const glm::vec3 &point) const override {
        float leftDistance = m_Left->Distance(point);
        float rightDistance = m_Right->Distance(point);

        switch (m_Operation) {
        case Operation::Union:
            return glm::min(leftDistance, rightDistance);
        case Operation::SmoothUnion:
            return SmoothMin(leftDistance, rightDistance, m_Smoothing);
        case Operation::Intersection:
            return glm::max(leftDistance, rightDistance);
        case Operation::SmoothIntersection:
            return SmoothMax(leftDistance, rightDistance, m_Smoothing);
        case Operation::Difference:
            return glm::max(leftDistance, -rightDistance);
        case Operation::SmoothDifference:
            return SmoothMax(leftDistance, -rightDistance, m_Smoothing);
        }
    }

  private:
    float SmoothMax(float a, float b, float k) const { return log2(exp2(k * a) + exp2(k * b)) / k; }
    float SmoothMin(float a, float b, float k) const {
        return -log2(exp2(-k * a) + exp2(-k * b)) / k;
    }

  private:
    float m_Smoothing = 1.0f;
    Operation m_Operation;
    std::shared_ptr<SDFGeometry> m_Left;
    std::shared_ptr<SDFGeometry> m_Right;
};
