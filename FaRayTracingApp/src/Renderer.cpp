#include "Renderer.h"
#include "Walnut/Random.h"

void Renderer::render()
{
	for (uint32_t y=0; y < m_final_image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_final_image->GetWidth(); x++)
		{
			glm::vec2 coordinates = { (float)x / (float)m_final_image->GetWidth(), (float)y / (float)m_final_image->GetHeight() };
			// convert from 0,1 to -1,1
			coordinates = coordinates * 2.0f - 1.0f;
			m_final_image_data[x + y*m_final_image->GetWidth()] = pixel_shader(coordinates);
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

uint32_t Renderer::pixel_shader(glm::vec2 coordinates)
{
	glm::vec3 ray_origin(0.0f, 0.0f, 2.0f);
	glm::vec3 ray_direction(coordinates.x, coordinates.y, -1.0f);
	ray_direction = glm::normalize(ray_direction);

	// compute eq coeffs
	float a = glm::dot(ray_direction, ray_direction);
	float b = 2.0f * glm::dot(ray_origin, ray_direction);
	float c = glm::dot(ray_origin, ray_origin) - m_radius * m_radius;

	// compute delta
	float delta_discriminant = b * b - 4.0f * a * c;
	
	// positive means more than one solution
	if (delta_discriminant >= 0)
	{
		return 0xffff00ff;
	}
	return 0xff000000;
}
