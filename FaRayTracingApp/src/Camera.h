#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
	Camera(float vertical_fov, float near_clip, float far_clip);

	void on_update(float time_step);
	void handle_size(uint32_t width, uint32_t height);

	const glm::mat4& get_projection() const { return m_projection; }
	const glm::mat4& get_inverse_projection() const { return m_inverse_projection; }
	const glm::mat4& get_view() const { return m_view; }
	const glm::mat4& get_inverse_view() const { return m_inverse_view; }

	const glm::vec3& get_position() const { return m_position; }
	const glm::vec3& get_direction() const { return m_forward_direction; }

	const std::vector<glm::vec3>& get_ray_directions() const { return m_ray_directions; }

	float get_rotation_speed();
private:
	void recalculate_projection();
	void recalculate_view();
	void recalculate_ray_directions();
private:
	static constexpr glm::vec3 m_up_direction = glm::vec3(0.0f, 1.0f, 0.0f);
	static constexpr float m_speed = 3.0f;

	glm::mat4 m_projection{ 1.0f };
	glm::mat4 m_view{ 1.0f };
	glm::mat4 m_inverse_projection{ 1.0f };
	glm::mat4 m_inverse_view{ 1.0f };

	float m_vertical_fov = 45.0f;
	float m_near_clip = 0.1f;
	float m_far_clip = 100.0f;

	glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_forward_direction{ 0.0f, 0.0f, 0.0f };

	// cached ray directions so that we save cpu compute
	std::vector<glm::vec3> m_ray_directions;

	glm::vec2 m_last_mouse_position{ 0.0f, 0.0f };

	uint32_t m_viewport_width = 0;
	uint32_t m_viewport_height = 0;
};