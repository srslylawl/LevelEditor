#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

struct VisualGrid {
	glm::mat4 transformMat = {};
	float gridSize;
	float cellSize;
};

