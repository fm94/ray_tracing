#include "Renderer.h"
#include <Walnut/Random.h>
#include "Utils.h"
#include <execution>

void Renderer::render(const Scene& scene, const Camera& camera)
{
	m_active_scene = &scene;
	m_active_camera = &camera;

	if (m_frame_index == 1)
		memset(m_accumulation_data, 0, m_final_image->GetWidth() * m_final_image->GetHeight() * sizeof(glm::vec4));

	// regarding mutli-threading, I tried all solutions proposed in the github issue but none seems to outperform simple for_each
#define MT 1
#if MT
	std::for_each(std::execution::par, m_image_v_iter.begin(), m_image_v_iter.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_image_h_iter.begin(), m_image_h_iter.end(),
			[this, y](uint32_t x)
				{
					glm::vec4 color = ray_gen(x, y);
					m_accumulation_data[x + y * m_final_image->GetWidth()] += color;
					glm::vec4 accumulated_color = m_accumulation_data[x + y * m_final_image->GetWidth()];
					accumulated_color /= (float)m_frame_index;
					accumulated_color = glm::clamp(accumulated_color, glm::vec4(0.0f), glm::vec4(1.0f));
					m_final_image_data[x + y * m_final_image->GetWidth()] = Utils::vec_to_rgba(accumulated_color);
				});
		});
#else
	for (uint32_t y=0; y < m_final_image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image->GetWidth(); x++)
		{
			glm::vec4 color = ray_gen(x, y);
			m_accumulation_data[x + y * m_final_image->GetWidth()] += color;
			glm::vec4 accumulated_color = m_accumulation_data[x + y * m_final_image->GetWidth()];
			accumulated_color /= (float)m_frame_index;
			accumulated_color = glm::clamp(accumulated_color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_final_image_data[x + y * m_final_image->GetWidth()] = Utils::vec_to_rgba(accumulated_color);
		}
	}
#endif

	m_final_image->SetData(m_final_image_data);

	if (m_settings.accumulate)
		m_frame_index++;
	else
		m_frame_index = 1;
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

	delete[] m_accumulation_data;
	m_accumulation_data = new glm::vec4[width * height];

	m_image_h_iter.resize(width);
	m_image_v_iter.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_image_h_iter[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_image_v_iter[i] = i;
}

std::shared_ptr<Walnut::Image> Renderer::get_final_image() const
{
	return m_final_image;
}

glm::vec4 Renderer::ray_gen(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.origin = m_active_camera->get_position();
	ray.direction = m_active_camera->get_ray_directions()[x + y * m_final_image->GetWidth()];

	glm::vec3 color(0.0f);
	float multiplier = 1.0f;

	int bounces = 3;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = trace_ray(ray);
		if (payload.hit_distance < 0.0f)
		{
			glm::vec3 sky_color = glm::vec3(0.6f, 0.7f, 0.9f);
			color += sky_color * multiplier;
			break;
		}

		// define a light source
		// think about it in the 3D space
		glm::vec3 light_src_ray = glm::normalize(glm::vec3(-1, 1, -1));
		// basically if light ray and normal are in reverse directions we get the best lighting
		// At 90d we have the least
		// more than that means we are at the other side so we just clamp with the max op because the dot is negative
		float light_intensity = glm::max(glm::dot(payload.world_normal, -light_src_ray), 0.0f);

		const Sphere& closest_sphere = m_active_scene->spheres[payload.object_index];
		// apply intensity on initial color using material
		const Material& material = m_active_scene->materials[closest_sphere.material_index];
		glm::vec3 sphere_color = material.albedo * light_intensity;
		color += sphere_color * multiplier;

		multiplier *= 0.7f;

		ray.origin = payload.world_position + payload.world_normal * 0.0001f;
		ray.direction = glm::reflect(ray.direction, payload.world_normal + material.roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::trace_ray(const Ray& ray)
{
	// look for the closest sphere
	int closest_sphere = -1;
	float current_hist_distance = FLT_MAX;
	for (size_t i = 0; i < m_active_scene->spheres.size(); i++) {
		const Sphere& sphere = m_active_scene->spheres[i];

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
		// also check if t is negative because that means the object is behind
		if (closest_t > 0.0f && closest_t < current_hist_distance) {
			current_hist_distance = closest_t;
			closest_sphere = (int)i;
		}
		//float other_t = (-b + glm::sqrt(delta_discriminant)) / 2 * a;
	}

	// if nothing has been hit then return black
	if (closest_sphere < 0) {
		return handle_miss();
	}

	return handle_closest_hit(ray, current_hist_distance, closest_sphere);
}

Renderer::HitPayload Renderer::handle_closest_hit(const Ray& ray, float hit_distance, int object_index)
{
	Renderer::HitPayload payload;
	payload.hit_distance = hit_distance;
	payload.object_index = object_index;

	const Sphere& closest_sphere = m_active_scene->spheres[object_index];

	// solving the equation
	glm::vec3 shifted_origin = ray.origin - closest_sphere.position;
	// typically here we consider the origin but it's 0
// update: this is now fixed
	payload.world_position = shifted_origin + ray.direction * hit_distance;
	payload.world_normal = glm::normalize(payload.world_position);
	// shift back but after normalization
	payload.world_position += closest_sphere.position;

	return payload;
}

Renderer::HitPayload Renderer::handle_miss()
{
	return HitPayload();
}
