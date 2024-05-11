#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>

#include <Walnut/Image.h>
#include <Walnut/Timer.h>

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h";

#include <glm/gtc/type_ptr.hpp>

#include "gif.h"

class MainLayer : public Walnut::Layer
{
public:
	MainLayer() : m_camera(45.0f, 0.1f, 100.0f)
	{
		
		//// create materials
		//m_scene.materials.emplace_back(Material{ { 1.0f, 0.0f, 0.0f }, 0.2f }); // red, mirror
		//m_scene.materials.emplace_back(Material{ { 0.0f, 0.0f, 1.0f }, 0.2f }); // blue, mirror
		//m_scene.materials.emplace_back(Material{ { 0.0f, 0.0f, 0.0f }, 0.4f }); // black, a bit rough
		//m_scene.materials.emplace_back(Material{ { 0.8f, 0.5f, 0.2f }, 0.1f , 0.0f, { 0.8f, 0.5f, 0.2f }, 2.0f}); // orange, lamp

		//// create spheres
		//m_scene.spheres.push_back(Sphere{ { -1.2f, 0.0f, 1.8f }, 1.0f, 0}); // red
		//m_scene.spheres.push_back(Sphere{ { 1.3f, 0.0f, -0.1f }, 1.0f, 1 }); // blue
		//m_scene.spheres.push_back(Sphere{ { 0.0f, 101.0f, 0.0f }, 100.0f, 2}); // black, acts as floor
		//m_scene.spheres.push_back(Sphere{ { 2.0f, 0.0f, 0.0f }, 1.0f, 2 }); // orange, acts as sun

		Material& pinkSphere = m_scene.materials.emplace_back();
		pinkSphere.albedo = { 1.0f, 0.5f, 0.5f };
		pinkSphere.roughness = 0.0f;

		Material& blueSphere = m_scene.materials.emplace_back();
		blueSphere.albedo = { 0.3f, 0.3f, 0.3f };
		blueSphere.roughness = 0.1f;

		Material& orangeSphere = m_scene.materials.emplace_back();
		orangeSphere.albedo = { 0.8f, 0.5f, 0.2f };
		orangeSphere.roughness = 0.1f;
		orangeSphere.emission_color = orangeSphere.albedo;
		orangeSphere.emission_power = 2.0f;

		Material& rSphere = m_scene.materials.emplace_back();
		rSphere.albedo = { 0.7f, 0.0f, 0.0f };
		rSphere.roughness = 0.4f;

		Material& gSphere = m_scene.materials.emplace_back();
		gSphere.albedo = { 0.0f, 0.5f, 0.0f };
		gSphere.roughness = 0.6f;

		Material& bSphere = m_scene.materials.emplace_back();
		bSphere.albedo = { 0.0f, 0.0f, 1.0f };
		bSphere.roughness = 0.8f;

		{
			Sphere sphere;
			sphere.position = { 0.0f, 0.0f, 0.0f };
			sphere.radius = 1.0f;
			sphere.material_index = 0;
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = { 1.4f, 6.7f, -9.6f };
			sphere.radius = 3.7f;
			sphere.material_index = 2;
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = { 0.0f, -101.0f, 0.0f };
			sphere.radius = 100.0f;
			sphere.material_index = 1;
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = { -2.1f, -0.4f, 0.8f };
			sphere.radius = 0.6f;
			sphere.material_index = 3;
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = { 2.6f, -0.7f, 0.4f };
			sphere.radius = 0.5f;
			sphere.material_index = 4;
			m_scene.spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.position = { -1.8f, 0.0f, -3.6f };
			sphere.radius = 1.0f;
			sphere.material_index = 5;
			m_scene.spheres.push_back(sphere);
		}
		// initial camera setup
		m_camera.custom_mov_rotation();
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
		ImGui::Text("Gif Frames: %d", m_gif_frames);
		ImGui::Text("Accumulate frames: %d", m_renderer.get_frame_index());
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
			ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.emission_color));
			ImGui::DragFloat("Emission Power", &material.emission_power, 0.05f, 0.0f, FLT_MAX);
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
			ImGui::Image(final_image->GetDescriptorSet(), { (float)final_image->GetWidth(), (float)final_image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
		//ImGui::PopStyleVar();

		Render();
		
		if (m_first_pass && m_renderer.get_frame_index() == 3) {
			const char* filename = "./demo.gif";
			GifBegin(&m_gif_writer, filename, final_image->GetWidth(), final_image->GetHeight(), 2, 8, true);
			m_first_pass = false;
		}
		if (m_renderer.get_frame_index() == 100000) {
			m_renderer.reset_frame_index();

			// fix this hack for flipping
			uint8_t* flipped_image = new uint8_t[final_image->GetWidth() * final_image->GetHeight() * 4];;
			flipAndConvertImage(m_renderer.get_frame_data(), flipped_image, final_image->GetWidth(), final_image->GetHeight());

			GifWriteFrame(&m_gif_writer, flipped_image, final_image->GetWidth(), final_image->GetHeight(), 2, 8, true);
			m_camera.custom_mov_rotation();
			m_gif_frames++;
			if (m_gif_frames == 30*50) {
				GifEnd(&m_gif_writer);
				delete[] flipped_image;
				std::exit(EXIT_SUCCESS);
			}
			delete[] flipped_image;
		}
	}

	void flipAndConvertImage(const uint32_t* constImage, uint8_t* flippedImage, int width, int height) {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				uint32_t pixelValue = constImage[y * width + x];
				flippedImage[((height - 1 - y) * width + x) * 4] = static_cast<uint8_t>((pixelValue) & 0xFF); // Red
				flippedImage[((height - 1 - y) * width + x) * 4 + 1] = static_cast<uint8_t>((pixelValue >> 8) & 0xFF);  // Green
				flippedImage[((height - 1 - y) * width + x) * 4 + 2] = static_cast<uint8_t>((pixelValue >> 16) & 0xFF);      // Blue
				flippedImage[((height - 1 - y) * width + x) * 4 + 3] = static_cast<uint8_t>((pixelValue >> 24) & 0xFF); // Alpha
			}
		}
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
	uint32_t m_viewer_width = 0;
	uint32_t m_viewer_height = 0;
	float m_last_render_time = 0;

	GifWriter m_gif_writer = {};
	int m_gif_frames{ 0 };
	bool m_first_pass = true;
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