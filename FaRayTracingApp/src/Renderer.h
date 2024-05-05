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
	glm::vec4 ray_gen(uint32_t x, uint32_t y);

	struct HitPayload
	{
		float hit_distance = -1.0f;
		glm::vec3 world_position;
		glm::vec3 world_normal;
		int object_index;
	};

	HitPayload trace_ray(const Ray& ray);
	HitPayload handle_closest_hit(const Ray& ray, float hit_distance, int object_index);
	HitPayload handle_miss();

private:
	std::shared_ptr<Walnut::Image> m_final_image;
	uint32_t* m_final_image_data = nullptr;
	const Scene* m_active_scene = nullptr;
	const Camera* m_active_camera = nullptr;

};