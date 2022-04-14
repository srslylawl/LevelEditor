#pragma once

#include "Input.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Time.h"


using namespace glm;

class Camera {
	std::vector<InputKeyBinding*> inputBindings;
	InputMouseBinding* mouseBinding = nullptr;
	bool relativeMouseModeActive = false;

	bool viewMatrixDirty = false;
public:
	vec3 Position = vec3(0, 0, -3.0f); //in World Space
	vec3 Target = vec3(0, 0, 0); // Target location in World Space
	vec3 Forward = vec3(0, 0, 1.0f);
	vec3 Right{}; //Relative to Camera
	vec3 Up{}; //Relative to Camera
	vec3 Rotation{};

	float yaw = -90;
	float pitch = 0;

	inline static float MoveSpeed = 50.0f;
	inline static float TurnSpeed = 50.0f;
	inline static float FOV = 45.0f;

	void Move(vec3 moveDirection) {
		Position += moveDirection * MoveSpeed * Time::GetDeltaTime();

		viewMatrixDirty = true;
	}

	void Rotate(int deltaX, int deltaY) {
		const auto relative = TurnSpeed * Time::GetDeltaTime();
		yaw -= deltaX * relative;
		pitch += deltaY * relative;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		Forward.x = cos(radians(yaw)) * cos(radians(pitch));
		Forward.y = sin(radians(pitch));
		Forward.z = sin(radians(yaw)) * cos(radians(pitch));

		Forward = -normalize(Forward);
		printf("Forward: (%f,%f,%f)\n", Forward.x, Forward.y, Forward.z);


		viewMatrixDirty = true;
	}

	void Update() {
		constexpr auto worldUp = vec3(0, 1.0f, 0);

		Right = normalize(cross(worldUp, Forward));
		Up = cross(Forward, Right);

		viewMatrixDirty = false;
	}

	mat4 GetViewMatrix() {
		if(viewMatrixDirty) this->Update();

		return lookAtLH(Position, Position + Forward, Up);
	}

	Camera() {
		inputBindings.push_back(Input::AddKeyBinding(SDLK_w, [this](KeyEvent e) {this->Move(this->Forward); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_s, [this](KeyEvent e) {this->Move(-this->Forward); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_a, [this](KeyEvent e) {this->Move(-this->Right); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_d, [this](KeyEvent e) {this->Move(this->Right); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_SPACE, [this](KeyEvent e){this->Move(this->Up); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_x, [this](KeyEvent e) {this->Move(-this->Up); }));
		mouseBinding = Input::AddMouseBinding([this](const InputMouseEvent* e){
			if(e->GetMouseKeyDown(MouseButton::Right)) {
				Input::SetMouseCapture(true);
				relativeMouseModeActive = true;
			}

			if(e-> GetMouseKeyUp(MouseButton::Right)) {
				Input::SetMouseCapture(false);
				relativeMouseModeActive = false;
			}

			if(e->GetMouseKeyHold(MouseButton::Right)) {
				this->Rotate(e->motion->deltaX, e->motion->deltaY);
			}
		});
		Update();
	}


	~Camera() {
		for (auto binding : inputBindings) {
			Input::RemoveKeyBinding(binding);
		}
		Input::RemoveMouseBinding(mouseBinding);

		if(relativeMouseModeActive) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
		}
	}
};


