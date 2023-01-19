#pragma once

#include <glm/glm.hpp>
#include <vector>

//Implement camera:
//Makes it easier to change/control field of view, size of sensor capturing light
//Easier to change position and rotation -> Interact with the camera
class Camera
{
public:
    //verticalFOV in degrees
    //near and far clipping planes of what our camera can see to create frostum -> anything outside frostum will be clipped = not rendered
    //Why clipped? Take data and convert it to -1, 1 space -> anything out of that range will not end up on screen
    Camera(float verticalFOV, float nearClip, float farClip);

    //Needs to run on every frame with timestamp ts -> move around with constant speed independent on frame rate
    bool OnUpdate(float ts);
    
    //Recalculate projection matrix
    void OnResize(uint32_t width, uint32_t height);

    //Getters for various details of camera
    const glm::mat4& GetProjection() const { return m_Projection; }
    const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
    const glm::mat4& GetView() const { return m_View; }
    const glm::mat4& GetInverseView() const { return m_InverseView; }

    //Where in space
    const glm::vec3& GetPosition() const { return m_Position; }
    //Where is it pointing
    const glm::vec3& GetDirection() const { return m_ForwardDirection; }

    //Convert camera projection matrices and view matrix, ... into ray directions and map to -1 and 1
    //On CPU might be slow --> will move to GPU and will be no problem
    //But for now we cache the directions so we dont have to recalculate when camera is not moving 
    const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

    float GetRotationSpeed();
private:
    void RecalculateProjection();
    void RecalculateView();
    void RecalculateRayDirections();
private:
    glm::mat4 m_Projection{ 1.0f };
    glm::mat4 m_View{ 1.0f };
    glm::mat4 m_InverseProjection{ 1.0f };
    glm::mat4 m_InverseView{ 1.0f };

    float m_VerticalFOV = 45.0f;
    float m_NearClip = 0.1f;
    float m_FarClip = 100.0f;

    glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
    glm::vec3 m_ForwardDirection{0.0f, 0.0f, 0.0f};

    // Cached ray directions
    std::vector<glm::vec3> m_RayDirections;

    glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

    uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};