#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Walnut/Input/Input.h>

using namespace Walnut;

Camera::Camera(float vertical_fov, float near_clip, float far_clip)
	: m_vertical_fov(vertical_fov), m_near_clip(near_clip), m_far_clip(far_clip)
{
	m_forward_direction = glm::vec3(0, 0, -1);
	m_position = glm::vec3(0, 0, 3);
}

void Camera::on_update(float time_step)
{
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 delta = (mousePos - m_last_mouse_position) * 0.002f;
	m_last_mouse_position = mousePos;

	if (!Input::IsMouseButtonDown(MouseButton::Right))
	{
		Input::SetCursorMode(CursorMode::Normal);
		return;
	}

	Input::SetCursorMode(CursorMode::Locked);

	bool moved = false;

	// we need to know where are we and where is right.
	// forward is cached, up is always the same -> easily get right
	
	// the trick with the right hand here is to flip the vectors instead of adding a - sign
	glm::vec3 right_direction = glm::cross(m_forward_direction, m_up_direction);

	// movement logic, can't press inverse keys simultaniously
	// TODO: i don't really like how it is right now with capturing mouse delta, maybe change it
	// forward
	if (Input::IsKeyDown(KeyCode::W))
	{
		m_position += m_forward_direction * m_speed * time_step;
		moved = true;
	}
	// backward
	else if (Input::IsKeyDown(KeyCode::S))
	{
		m_position -= m_forward_direction * m_speed * time_step;
		moved = true;
	}
	// left
	if (Input::IsKeyDown(KeyCode::A))
	{
		m_position -= right_direction * m_speed * time_step;
		moved = true;
	}
	// right
	else if (Input::IsKeyDown(KeyCode::D))
	{
		m_position += right_direction * m_speed * time_step;
		moved = true;
	}
	// down
	if (Input::IsKeyDown(KeyCode::Q))
	{
		m_position -= m_up_direction * m_speed * time_step;
		moved = true;
	}
	// up
	else if (Input::IsKeyDown(KeyCode::E))
	{
		m_position += m_up_direction * m_speed * time_step;
		moved = true;
	}

	// rotation logic
	// the entire idea is that a rotation gives us a new forward direction and from that everything else can be deducted
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitch_delta = delta.y * get_rotation_speed(); // rotation up-down
		float yaw_delta = delta.x * get_rotation_speed(); // rotation right-left

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(pitch_delta, right_direction),
			glm::angleAxis(yaw_delta, glm::vec3(0.f, 1.0f, 0.0f))));
		m_forward_direction = glm::rotate(q, m_forward_direction);

		moved = true;
	}

	if (moved)
	{
		recalculate_view();
		recalculate_ray_directions();
	}
}

void Camera::handle_size(uint32_t width, uint32_t height)
{
	if (width == m_viewport_width && height == m_viewport_height)
		return;

	m_viewport_width = width;
	m_viewport_height = height;

	recalculate_projection();
	recalculate_ray_directions();
}

float Camera::get_rotation_speed()
{
	return 0.3f;
}

void Camera::recalculate_projection()
{
	m_projection = glm::perspectiveFov(glm::radians(m_vertical_fov), (float)m_viewport_width, (float)m_viewport_height, m_near_clip, m_far_clip);
	m_inverse_projection = glm::inverse(m_projection);
}

void Camera::recalculate_view()
{
	m_view = glm::lookAt(m_position, m_position + m_forward_direction, m_up_direction);
	m_inverse_view = glm::inverse(m_view);
}

void Camera::recalculate_ray_directions()
{
	m_ray_directions.resize(m_viewport_width * m_viewport_height);

	for (uint32_t y = 0; y < m_viewport_height; y++)
	{
		for (uint32_t x = 0; x < m_viewport_width; x++)
		{
			glm::vec2 coord = { (float)x / (float)m_viewport_width, (float)y / (float)m_viewport_height };
			coord = coord * 2.0f - 1.0f; // -1 -> 1

			glm::vec4 target = m_inverse_projection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_inverse_view * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_ray_directions[x + y * m_viewport_width] = rayDirection;
		}
	}
}