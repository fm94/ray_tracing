#pragma once

#include "Walnut/Image.h"
#include "memory"
#include "glm/glm.hpp"

class Renderer
{
public:
	static constexpr float m_radius = 0.5f;

	Renderer() = default;

	void render();
	void handle_size(uint32_t width, uint32_t height);
	std::shared_ptr<Walnut::Image> get_final_image() const;

private:
	glm::vec4 pixel_shader(glm::vec2 coordinates);

private:


	std::shared_ptr<Walnut::Image> m_final_image;
	uint32_t* m_final_image_data = nullptr;

};