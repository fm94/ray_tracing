#pragma once

#include <Walnut/Image.h>
#include <memory>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"


class Renderer
{
public:
	Renderer() = default;

	void render(const Scene& scene, const Camera& camera);
	void handle_size(uint32_t width, uint32_t height);
	std::shared_ptr<Walnut::Image> get_final_image() const;

private:
	glm::vec4 trace_ray(const Scene& scene, const Ray& ray);

private:


	std::shared_ptr<Walnut::Image> m_final_image;
	uint32_t* m_final_image_data = nullptr;

};