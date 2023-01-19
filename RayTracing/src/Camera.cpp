#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip)
{
	//Set direction of where camera is looking and set its position in space
	m_ForwardDirection = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 5);
}

bool Camera::OnUpdate(float ts)
{
	//Get delta of mouse = how much mouse has moved in 1 frame
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f; //0.002f = sensitivity of mouse
	m_LastMousePosition = mousePos;

	//If right mouse button is not pressed -> Return and do nothing
	if (!Input::IsMouseButtonDown(MouseButton::Right))
	{
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}
	//If right mouse is pressed -> lock cursor to window, we can still get delta but cursor wont be visible
	Input::SetCursorMode(CursorMode::Locked);	

	//bool flag to tell if we moved the camera and need to calculate the ray directions and matrices 
	bool moved = false;

	//Get right direction of camera by calculating the crossproduct between the up direction and forward direction
	//Cross product will return a direction perpendicular to both input directions
	//Use right direction for movement
	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f); //Up direction is always just straight up, dont support tilting
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	//Speed factor of camera movement
	float speed = 5.0f;
	//ts factor for everything -> scale accordingly to how fast program is running 
	// Movement
	//Check if camera has moved
	if (Input::IsKeyDown(KeyCode::W))
	{
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::S))
	{
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::A))
	{
		m_Position -= rightDirection * speed * ts;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::D))
	{
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::Q))
	{
		m_Position -= upDirection * speed * ts;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::E))
	{
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	// Check if camera has rotated 
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		//Calculate how much we have to rotate (constant rotation speed)
		//Rotation along x-axis - up and down -> corresponds to y axis mouse movement
		float pitchDelta = delta.y * GetRotationSpeed();
		//Rotation along y-axis - left and right -> corresponds to x axis mouse movement
		float yawDelta = delta.x * GetRotationSpeed();

		//Calculate how much we rotated in each axis and get new forward direction by taking pitch and yaw delta into account
		//Delta of the entire rotation 
		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		//Rotate forward direction by calculated amount that we rotated
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved)
	{
		RecalculateView();
		RecalculateRayDirections();
	}
	return moved;
}

//Recalculate all things dependent on width and height if camera gets resized
void Camera::OnResize(uint32_t width, uint32_t height)
{
	if (width == m_ViewportWidth && height == m_ViewportHeight)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

float Camera::GetRotationSpeed()
{
	return 0.3f;
}

//Creates perspective matrix for us based on FOV, width and height (aspect ratio) , near and far clip
//Creates the frustum and maps the coords in our space to -1 and 1 to be able to display on screen
void Camera::RecalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	//Get inverse projection -> useful later
	m_InverseProjection = glm::inverse(m_Projection);
}

//Builds up view matrix
void Camera::RecalculateView()
{
	//lookAt -> "Hey you are here and look at this point"
	//Specify position, what point to look at and and up direction
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	//Inverting matrix to get inverse view -> will be useful in future
	m_InverseView = glm::inverse(m_View);
}

//Cached calculations
//Figure out ray directions from view and projection matrix
void Camera::RecalculateRayDirections()
{
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++)
	{
		for (uint32_t x = 0; x < m_ViewportWidth; x++)
		{
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
			coord = coord * 2.0f - 1.0f; // Calculate -1 -> 1 coordinate
			
			//Here in the target we go back to world space from -1 -> 1
			//We know the -1 -> 1 space = the pixels
			//We need to do the opposite: we need to cast our ray in world space but we have -1 -> 1
			//Need to reverse -> multiply with inverse of each matrix
			//Target  = where we are targeting this vector
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			//Ray direction = inverse view * perspective division of target normalized
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			//Cache results in vector
			m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}