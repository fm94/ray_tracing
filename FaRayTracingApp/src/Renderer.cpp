#include "Renderer.h"
#include "Walnut/Random.h"
#include "utils.h"

void Renderer::render()
{
	for (uint32_t y=0; y < m_final_image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image->GetWidth(); x++)
		{
			glm::vec2 coordinates = { (float)x / (float)m_final_image->GetWidth(), (float)y / (float)m_final_image->GetHeight() };
			// convert from 0,1 to -1,1
			coordinates = coordinates * 2.0f - 1.0f;

			// basically get the color as vec and convert it to 32 bit uint
			glm::vec4 pixel_color_vec = pixel_shader(coordinates);
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

glm::vec4 Renderer::pixel_shader(glm::vec2 coordinates)
{
	glm::vec3 ray_origin(0.0f, 0.0f, 1.0f);
	glm::vec3 ray_direction(coordinates.x, coordinates.y, -1.0f);
	ray_direction = glm::normalize(ray_direction);

	// compute eq coeffs
	float a = glm::dot(ray_direction, ray_direction);
	float b = 2.0f * glm::dot(ray_origin, ray_direction);
	float c = glm::dot(ray_origin, ray_origin) - m_radius * m_radius;

	// compute delta
	float delta_discriminant = b * b - 4.0f * a * c;
	
	// negative means no hit
	if (delta_discriminant < 0) 
	{
		return glm::vec4(0, 0, 0, 1);
	}
	// otherwise we have two solutions, one of them is the one facing our camera
	// tthe one with the shortest distance is the relevant one here
	float closest_t = (-b - glm::sqrt(delta_discriminant)) / 2 * a;
	float other_t = (-b + glm::sqrt(delta_discriminant)) / 2 * a;
	// solving the equation
	glm::vec3 nearest_hit_point = ray_origin + ray_direction * closest_t;
	glm::vec3 nearest_hit_point_normal = glm::normalize(nearest_hit_point); // typically here we consider the origin but it's 0

	// define a light source
	// think about it in the 3D space
	glm::vec3 light_src_ray = glm::normalize(glm::vec3(-1, -1, -1)); 
	// basically if light ray and normal are in reverse directions we get the best lighting
	// At 90d we have the least
	// more than that means we are at the other side so we just clamp with the max op because the dot is negative
	float light_intensity = glm::max(glm::dot(nearest_hit_point_normal, -light_src_ray), 0.0f);
	
	// apply intensity on initial color
	glm::vec3 sphere_color = glm::vec3(1, 0, 1) * light_intensity;
	return glm::vec4(sphere_color, 1.0f);
}
