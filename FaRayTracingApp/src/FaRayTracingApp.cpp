#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

class MainLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("FPS: %.3f", 1000 / m_last_render_time);
		if (ImGui::Button("Render")) {
			Render();
		}
		ImGui::End();

		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Viewer");
		m_viewer_width = ImGui::GetContentRegionAvail().x;
		m_viewer_height = ImGui::GetContentRegionAvail().y;
		auto final_image = m_renderer.get_final_image();
		if (final_image) {
			ImGui::Image(final_image->GetDescriptorSet(), { (float)final_image->GetWidth(), (float)final_image->GetHeight() });
		}
		ImGui::End();

		Render();
	}

	void Render() {
		Walnut::Timer timer;

		m_renderer.handle_size(m_viewer_width, m_viewer_height);
		m_renderer.render();

		m_last_render_time = timer.ElapsedMillis();
	}
private:
	Renderer m_renderer;
	uint32_t m_viewer_width;
	uint32_t m_viewer_height;
	float m_last_render_time = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "FaRay Tracing Tests";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<MainLayer>();
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