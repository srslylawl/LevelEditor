#pragma once

#include "Input.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Time.h"


using namespace glm;

class Camera {
	std::vector<InputActionBinding*> inputBindings;
	InputMouseBinding* mouseBinding = nullptr;
public:
	vec3 Position = vec3(0, 0, -3.0f); //in World Space
	vec3 Target = vec3(0, 0, 0); // Target location in World Space
	vec3 LookDirection = vec3(0, 0, 1.0f); //LookDirection to Target
	vec3 Right{}; //Relative to Camera
	vec3 Up{}; //Relative to Camera
	vec3 Rotation{};

	inline static float MoveSpeed = 50.0f;
	inline static float TurnSpeed = 0.5f;

	void Move(vec3 moveDirection) {
		Position += moveDirection * MoveSpeed * Time::GetDeltaTime();
	}

	void Rotate(int deltaX, int deltaY) {
		printf("RotateXY: %i, %i\n", deltaX, deltaY);
	}

	void Update() {
		constexpr auto worldUp = vec3(0, 1.0f, 0);

		Target = Position + vec3(0, 0, 1.0f);
		LookDirection = normalize(Target - Position);
		Right = normalize(cross(worldUp, LookDirection));
		Up = cross(LookDirection, Right);
	}

	mat4 GetViewMatrix() {
		this->Update();
		return lookAtLH(Position, Target, Up);
	}

	Camera() {
		inputBindings.push_back(Input::AddKeyBinding(SDLK_w, [this](KeyEvent e) {this->Move(this->LookDirection); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_s, [this](KeyEvent e) {this->Move(-this->LookDirection); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_a, [this](KeyEvent e) {this->Move(-this->Right); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_d, [this](KeyEvent e) {this->Move(this->Right); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_SPACE, [this](KeyEvent e){this->Move(this->Up); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_x, [this](KeyEvent e) {this->Move(-this->Up); }));
		mouseBinding = Input::AddMouseMovementBinding([this](const MouseMovementEvent* e){
			if(e->mouseButtonsHeldDown->count(SDL_BUTTON_RIGHT)) {
				this->Rotate(e->MouseDelta_X, e->MouseDelta_Y);
			}
		});
		Update();
	}


	~Camera() {
		for (auto binding : inputBindings) {
			Input::RemoveKeyBinding(binding);
		}
		Input::RemoveMouseBinding(mouseBinding);
	}
};


