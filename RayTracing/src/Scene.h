#pragma once
#include <vector>
#include <glm/vec3.hpp>

struct Material
{
    //Color = Albedo = Non lit diffuse component of a material
    glm::vec3 Albedo {1.0f};
    float Roughness = 1.0f;
    float Metallic = 0.0f;
};
struct Sphere
{
    glm::vec3 Position {0.0f};
    float Radius = 0.5f;
    int MaterialIndex = 0;
};
struct Scene
{
    std::vector<Sphere> Spheres; 
    std::vector<Material> Materials; 
};
