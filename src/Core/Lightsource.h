#pragma once

#include "Types.h"

#include <glm/glm.hpp>

class Light
{

public:
    enum class LightType {
        Directional,
        Point,
        Spotlight
    };

    virtual ~Light() = default;
    virtual LightType GetType() const = 0;

    glm::vec3 Color { 1.0f };
    float Intensity { 1.0f };

};

class DirectionalLight : public Light
{

public:
    glm::vec3 Direction { -1.0f, -1.0f, -1.0f };

    LightType GetType() const override
    {
        return LightType::Directional;
    }

};

class PointLight : public Light
{

public:
    glm::vec3 Position { 0.0f };

    float Constant  { 1.0f };
    float Linear    { 0.09f };
    float Quadratic { 0.032f };

    LightType GetType() const override
    {
        return LightType::Point;
    }

};

class SpotLight : public Light
{

public:
    glm::vec3 Position  { 0.0f };
    glm::vec3 Direction { 0.0f, -1.0f, 0.0f };

    float Constant  { 1.0f };
    float InnerCutoff { 12.5f };
    float OuterCutoff { 17.5f };

    LightType GetType() const override
    {
        return LightType::Spotlight;
    }

};