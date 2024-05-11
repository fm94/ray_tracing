#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material
{
	glm::vec3 albedo{ 1.0f }; // which defines the color of a sphere
	float roughness = 1.0f;   // how's the surface reflection
	float metallic = 0.0f;
	glm::vec3 emission_color{ 0.0f };
	float emission_power = 0.0f;

	glm::vec3 get_emission() const { return emission_color * emission_power; }
};

struct Sphere {
	glm::vec3 position{0.0f};
	float radius = 0.5f;
	int material_index = 0; // each sphere has a material
};

struct Scene {
	std::vector<Sphere> spheres;
	std::vector<Material> materials; // list of all available materials
};