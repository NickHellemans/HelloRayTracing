#pragma once 
#include "Walnut/Image.h"
#include <memory>
#include <glm/glm.hpp>
#include "Ray.h"
#include "Camera.h"
#include "Scene.h"

class Renderer
{
public:
    struct Settings
    {
        bool Accumulate = true;
    };
    
    Renderer() = default;
    void OnResize(uint32_t width, uint32_t height);
    void Render(const Scene& scene, const Camera& camera);
    std::shared_ptr<Walnut::Image> GetFinalImage() const {return m_FinalImage;}
    void ResetFrameIndex(){ m_FrameIndex = 1; }
    Settings& GetSettings(){ return m_Settings; }
    
private:
    struct HitPayload
    {
        float HitDistance;
        glm::vec3 WorldPosition;
        glm::vec3 WorldNormal;
        
        int ObjectIndex;
    };
    const Scene* m_ActiveScene = nullptr;
    const Camera* m_ActiveCamera = nullptr;
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData = nullptr;
    glm::vec4* m_AccumulationData = nullptr;
    Settings m_Settings;
    uint32_t m_FrameIndex = 1;
    //Basicly like a shader: Return a color per pixel from viewport based on coord in viewport
    //glm::vec4 PerPixel(glm::vec2 coord);
    
    glm::vec4 PerPixel(uint32_t x, uint32_t y); //RayGen shader - runs for every pixel we want to render, so we can choose when to call TraceRay and when not, will return the color
    HitPayload  TraceRay(const Ray& ray); //Shoots rays returns payload with info about what happened to the ray
    HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex); //Shader to run when we hit something
    HitPayload Miss(const Ray& ray); //Shader that runs when we dont hit anything
};
