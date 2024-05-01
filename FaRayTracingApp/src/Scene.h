#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Sphere {
	glm::vec3 position{0.0f, 0.0f, 0.0f};
	float radius = 0.5f;
	glm::vec3 albedo{ 1.0f }; // which defines the color of a sphere
};

struct Scene {
	std::vector<Sphere> spheres;
};