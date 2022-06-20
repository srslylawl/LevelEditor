#pragma once

#include <glm/vec3.hpp>

struct Ray {
	Ray(const glm::vec3& origin, const glm::vec3& direction)
		: Origin(origin),
		  Direction(direction) {}

	glm::vec3 Origin;
	glm::vec3 Direction;
};
