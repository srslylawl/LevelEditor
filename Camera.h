#pragma once

#include "Input.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Time.h"


using namespace glm;

class Camera {
	std::vector<InputActionBinding> inputBindings;
public:
	vec3 Position = vec3(0, 0, -3.0f); //in World Space
	vec3 Target = vec3(0, 0, 0); // Target location in World Space
	vec3 Direction = vec3(0, 0, 1.0f); //Direction to Target -- REVERSED
	vec3 Right{}; //Relative to Camera
	vec3 Up{}; //Relative to Camera

	inline static float CameraSpeed = 50.0f;

	void Move(vec3 moveDirection) {
		Position += moveDirection * CameraSpeed * Time::GetDeltaTime();
	}

	void Update() {
		constexpr auto worldUp = vec3(0, 1.0f, 0);

		Target = Position + vec3(0, 0, 1.0f);
		Direction = normalize(Target - Position);
		Right = normalize(cross(worldUp, Direction));
		Up = cross(Direction, Right);
	}

	mat4 GetViewMatrix() {
		this->Update();
		return lookAt(Position, Target, Up);
	}

	Camera() {
		inputBindings.push_back(Input::AddBinding(SDLK_w, [this](KeyEvent e) {this->Move(this->Direction); }));
		inputBindings.push_back(Input::AddBinding(SDLK_s, [this](KeyEvent e) {this->Move(-this->Direction); }));
		inputBindings.push_back(Input::AddBinding(SDLK_a, [this](KeyEvent e) {this->Move(this->Right); }));
		inputBindings.push_back(Input::AddBinding(SDLK_d, [this](KeyEvent e) {this->Move(-this->Right); }));
		inputBindings.push_back(Input::AddBinding(SDLK_SPACE, [this](KeyEvent e){this->Move(this->Up); }));
		inputBindings.push_back(Input::AddBinding(SDLK_x, [this](KeyEvent e) {this->Move(-this->Up); }));
		Update();
	}


	~Camera() {
		for (auto binding : inputBindings) {
			Input::RemoveBinding(&binding);
		}
	}
};


