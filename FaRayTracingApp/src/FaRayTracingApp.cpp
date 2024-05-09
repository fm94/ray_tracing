#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>

#include <Walnut/Image.h>
#include <Walnut/Timer.h>

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h";

#include <glm/gtc/type_ptr.hpp>

class MainLayer : public Walnut::Layer
{
public:
	MainLayer() : m_camera(45.0f, 0.1f, 100.0f)
	{
		// create materials
		m_scene.materials.emplace_back(Material{ { 1.0f, 0.0f, 0.0f }, 0.2f }); // red, mirror
		m_scene.materials.emplace_back(Material{ { 0.0f, 0.0f, 1.0f }, 0.2f }); // blue, mirror
		m_scene.materials.emplace_back(Material{ { 0.0f, 0.0f, 0.0f }, 0.4f }); // black, a bit rough

		// create spheres
		m_scene.spheres.push_back(Sphere{ { -1.2f, 0.0f, 1.8f }, 1.0f, 0}); // red
		m_scene.spheres.push_back(Sphere{ { 1.3f, 0.0f, -0.1f }, 1.0f, 1 }); // blue
		m_scene.spheres.push_back(Sphere{ { 0.0f, 101.0f, 0.0f }, 100.0f, 2}); // black, acts as floor
	}

	virtual void OnUpdate(float time_step) override
	{
		if (m_camera.on_update(time_step)) m_renderer.reset_frame_index();
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		//ImGui::Text("FPS: %.3f", 1000 / m_last_render_time);
		ImGui::Text("TPF: %.f ms", m_last_render_time);
		//if (ImGui::Button("Render")) {
		//	Render();
		//}
		ImGui::Checkbox("Accumulate", &m_renderer.get_settings().accumulate);
		if (ImGui::Button("Reset"))
			m_renderer.reset_frame_index();
		ImGui::End();

		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_scene.spheres.size(); i++) {
			ImGui::PushID(i);
			Sphere& sphere = m_scene.spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
			ImGui::DragInt("Material", &sphere.material_index, 1.0f, 0, (int)m_scene.materials.size() - 1);
			ImGui::Separator();
			ImGui::PopID();
		}

		for (size_t i = 0; i < m_scene.materials.size(); i++)
		{
			ImGui::PushID(i);
			Material& material = m_scene.materials[i];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
			ImGui::DragFloat("Roughness", &material.roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.metallic, 0.05f, 0.0f, 1.0f);
			ImGui::Separator();
			ImGui::PopID();
		}
		ImGui::End();


		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Viewer");
		m_viewer_width = (int)ImGui::GetContentRegionAvail().x;
		m_viewer_height = (int)ImGui::GetContentRegionAvail().y;
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
		m_camera.handle_size(m_viewer_width, m_viewer_height);
		m_renderer.render(m_scene, m_camera);

		m_last_render_time = timer.ElapsedMillis();
	}
private:
	Renderer m_renderer;
	Camera m_camera;
	Scene m_scene;
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