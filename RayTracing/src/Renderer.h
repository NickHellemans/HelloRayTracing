#pragma once 
#include "Walnut/Image.h"
#include <memory>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

class Renderer
{
public:
    Renderer()=default;
    void OnResize(uint32_t width, uint32_t height);
    void Render();
    std::shared_ptr<Walnut::Image> GetFinalImage() const {return m_FinalImage;}
private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData = nullptr;
    //Basicly like a shader: Return a color per pixel from viewport based on coord in viewport
    glm::vec4 PerPixel(glm::vec2 coord);
};
