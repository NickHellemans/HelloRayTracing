#include "Camera.h"
#include "Renderer.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		:m_Camera(45.0f,0.1f,100.f)
	{
		{
			Sphere sphere;
			sphere.Position = {0.0f, 0.0f, 0.0f};
			sphere.Radius = 0.5f;
			sphere.Albedo = {1.0f, 0.0f, 1.0f};
			m_Scene.Spheres.push_back(sphere);
		}
		
		{
			Sphere sphere;
			sphere.Position = {1.0f, 0.0f, -5.0f};
			sphere.Radius = 1.5f;
			sphere.Albedo = {0.2f, 0.3f, 1.0f};
			m_Scene.Spheres.push_back(sphere);
		}
	}
	
	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);	
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		if (ImGui::Button("Render")) {
			Render();
		}
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::End();

		//Scene controls
		ImGui::Begin("Scene");
		for(size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			//Create unique id for controls - else imgui will change all spheres because it does not know which label to target
			ImGui::PushID(i);
			
			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo));
			
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();

		//Push style var to get rid of border
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//Viewport where we can see rendered scene
		ImGui::Begin("Viewport");
		//Get current width and height from viewport
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight= ImGui::GetContentRegionAvail().y;

		//Display image if it exists
		auto image = m_Renderer.GetFinalImage();
		if(image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(),
				(float)image->GetHeight()},ImVec2(0,1), ImVec2(1,0));
		//uv0 and uv1 set origin of image: default (0,0) -> to flip image uv0 = (0,1) flip y, uv1 = (1,0) flip x
		ImGui::End();
		//Pop style var once window ends
		ImGui::PopStyleVar();

		Render();
	}

	void Render() {
		Timer timer;
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		//Pass a camera to renderer , instead of having it in renderer itself -> dont want renderer to control where we render from
		//Pass as a viewport to render from
		m_Renderer.Render(m_Scene, m_Camera);
		//Set timer to see how long it took to render
		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
	Camera m_Camera;
	Scene m_Scene; 
};

//Walnut app entry point
Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}