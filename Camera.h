#pragma once
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Camera
{
public:
	vec3 Position = vec3(0, 0, 3.0f);
	vec3 Direction = vec3(0, 0, 1.0f);
	vec3 Right = vec3(1.0f, 0, 0);
	vec3 Up = vec3(0, 1.0f, 0);

	void Move(vec3 moveDirection) {
		Position += moveDirection;
	}

	mat4 GetViewMatrix() const {
		return lookAt(Position, Direction, Up);
	}
};

