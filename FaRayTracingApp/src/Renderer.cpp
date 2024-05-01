#include "Renderer.h"
#include <Walnut/Random.h>
#include "Utils.h"

void Renderer::render(const Scene& scene, const Camera& camera)
{
	Ray ray;
	ray.origin = camera.get_position();

	for (uint32_t y=0; y < m_final_image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image->GetWidth(); x++)
		{
			ray.direction = camera.get_ray_directions()[x + y * m_final_image->GetWidth()];

			// basically get the color as vec and convert it to 32 bit uint
			glm::vec4 pixel_color_vec = trace_ray(scene, ray);
			pixel_color_vec = glm::clamp(pixel_color_vec, glm::vec4(0.0f), glm::vec4(1.0f));
			m_final_image_data[x + y * m_final_image->GetWidth()] = Utils::vec_to_rgba(pixel_color_vec);
		}
	}
	m_final_image->SetData(m_final_image_data);
}

void Renderer::handle_size(uint32_t width, uint32_t height)
{
	if (m_final_image)
	{
		// if the image size didn't change, no need to recreate the data buffer
		if (m_final_image->GetWidth() == width && m_final_image->GetHeight() == height)
			return;
		m_final_image->Resize(width, height);
	}
	else
	{
		m_final_image = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] m_final_image_data;
	m_final_image_data = new uint32_t[width * height];
	
}

std::shared_ptr<Walnut::Image> Renderer::get_final_image() const
{
	return m_final_image;
}

glm::vec4 Renderer::trace_ray(const Scene& scene, const Ray& ray)
{
	if (scene.spheres.size() == 0)
	{
		return glm::vec4(0, 0, 0, 1);
	}

	// look for the closest sphere
	const Sphere* closest_sphere = nullptr;
	float current_hist_distance = FLT_MAX;
	for (const Sphere& sphere : scene.spheres) {
		// compensate shifted circles (this is recalculated at the end, fix this)
		glm::vec3 shifted_origin = ray.origin - sphere.position;

		// compute eq coeffs
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(shifted_origin, ray.direction);
		float c = glm::dot(shifted_origin, shifted_origin) - sphere.radius * sphere.radius;

		// compute delta
		float delta_discriminant = b * b - 4.0f * a * c;

		// negative means no hit so just continue to next
		if (delta_discriminant < 0.0f)
		{
			continue;
		}
		// otherwise we have two solutions, one of them is the one facing our camera
		// tthe one with the shortest distance is the relevant one here
		float closest_t = (-b - glm::sqrt(delta_discriminant)) / 2 * a;
		if (closest_t < current_hist_distance) {
			current_hist_distance = closest_t;
			closest_sphere = &sphere;
		}
		//float other_t = (-b + glm::sqrt(delta_discriminant)) / 2 * a;
	}

	// if nothing has been hit then return black
	if (closest_sphere == nullptr) {
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// solving the equation
	glm::vec3 shifted_origin = ray.origin - closest_sphere->position;
	glm::vec3 nearest_hit_point = shifted_origin + ray.direction * current_hist_distance;
	glm::vec3 nearest_hit_point_normal = glm::normalize(nearest_hit_point); // typically here we consider the origin but it's 0

	// define a light source
	// think about it in the 3D space
	glm::vec3 light_src_ray = glm::normalize(glm::vec3(-1, -1, -1)); 
	// basically if light ray and normal are in reverse directions we get the best lighting
	// At 90d we have the least
	// more than that means we are at the other side so we just clamp with the max op because the dot is negative
	float light_intensity = glm::max(glm::dot(nearest_hit_point_normal, -light_src_ray), 0.0f);
	
	// apply intensity on initial color
	glm::vec3 sphere_color = closest_sphere->albedo * light_intensity;
	return glm::vec4(sphere_color, 1.0f);
}
