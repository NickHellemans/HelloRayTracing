#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		if (ImGui::Button("Render")) {
			Render();
		}
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::End();

		//Push style var to get rid of border
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//Viewport where we can see rendered scene
		ImGui::Begin("Viewport");
		//Get current width and height from viewport
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight= ImGui::GetContentRegionAvail().y;

		//Display image if it exists
		if(m_Image)
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });
		ImGui::End();
		//Pop style var once window ends
		ImGui::PopStyleVar();

		Render();
	}

	void Render() {
		Timer timer;
		//Create image if it doesnt exist or recreate it when width/height dont match current image size
		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportWidth != m_Image->GetHeight())
		{
			m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);
			//Delete current image data if exists
			delete[] m_ImageData; 
			//Reallocate for new data
			//uint32 is 32 bits -> RGBA 1 byte for each color = 32bits
			m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
		}

		//fill image data
		for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++) {
			m_ImageData[i] = Random::UInt(); // 0xffff00ff : From l->r: ABGR, 1 byte each == 2 letters
			m_ImageData[i] |= 0xff000000; //Or equals to set bits of alpha channel to 1 (most significant) because random uint will give us random alpha values aswell and we want to see image

		}

		//Upload to gpu for rendering - Inefficient to load data on cpu and transfer to GPU but will change later in series
		m_Image->SetData(m_ImageData);

		//Set timer to see how long it took to render
		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	std::shared_ptr<Image> m_Image;
	uint32_t* m_ImageData = nullptr; 
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
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