#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

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
		if (m_image) {
			ImGui::Image(m_image->GetDescriptorSet(), { (float)m_image->GetWidth(), (float)m_image->GetHeight() });
		}
		ImGui::End();

		Render();
	}

	void Render() {
		Walnut::Timer timer;

		if (!m_image || m_viewer_width != m_image->GetWidth() || m_viewer_height != m_image->GetHeight()) {
			m_image = std::make_shared<Walnut::Image>(m_viewer_width, m_viewer_height, Walnut::ImageFormat::RGBA);

			delete[] m_image_data;
			m_image_data = new uint32_t[m_viewer_width * m_viewer_height];
		}

		for (uint32_t index = 0; index < m_viewer_width * m_viewer_height; index++) {
			m_image_data[index] = Walnut::Random::UInt();
			m_image_data[index] |= 0xff000000;
		}

		m_image->SetData(m_image_data);
		m_last_render_time = timer.ElapsedMillis();
	}
private:
	std::shared_ptr<Walnut::Image> m_image;
	uint32_t m_viewer_width;
	uint32_t m_viewer_height;
	uint32_t* m_image_data = nullptr;
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