#pragma once

#include "Input.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Time.h"


using namespace glm;

enum class ViewMode {
	Perspective = 0,
	Orthographic = 1
};

class Camera {
	std::vector<InputKeyBinding*> inputBindings;
	InputMouseBinding* mouseBinding = nullptr;
	bool relativeMouseModeActive = false;

	bool viewMatrixDirty = true;

	mat4 projectionMatrix = {};
	mat4 viewMat = {};

	ViewMode viewMode = ViewMode::Perspective;

	int width;
	int height;
	float aspectRatio;
	float zNear = 0.0001f; // HAS to be >0
	float zFar = 200.0f;
	float fov = 45.0f;

	float zoom = 1;

	void UpdateProjectionMatrix() {
		if ( viewMode == ViewMode::Perspective) {
			aspectRatio = width / static_cast<float>(height);
			projectionMatrix = perspectiveLH(radians(fov/zoom), aspectRatio, zNear, zFar);
			return;
		}

		if ( viewMode == ViewMode::Orthographic) {
			const float halfW = width / 2.0f / zoom;
			const float halfH = height / 2.0f / zoom;
			projectionMatrix = orthoLH(-halfW, halfW, -halfH, halfH, zNear, zFar);
			return;
		}
	}

	vec3 position = vec3(0, 0, -3.0f); //in World Space
public:
	inline static Camera* Main = nullptr;
	vec3 Forward = vec3(0, 0, 1.0f);
	vec3 Right{}; //Relative to Camera
	vec3 Up{}; //Relative to Camera
	vec3 Rotation{};

	float yaw = -90;
	float pitch = 0;

	inline static float MoveSpeed = 50.0f;
	inline static float TurnSpeed = 50.0f;

	void Move(vec3 moveDirection) {
		position += moveDirection * MoveSpeed * Time::GetDeltaTime();

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
		//printf("Forward: (%f,%f,%f)\n", Forward.x, Forward.y, Forward.z);

		viewMatrixDirty = true;
	}

	void Update() {
		constexpr auto worldUp = vec3(0, 1.0f, 0);

		Right = normalize(cross(worldUp, Forward));
		Up = cross(Forward, Right);

		viewMat = lookAtLH(position, position + Forward, Up);
		viewMatrixDirty = false;
	}

	void SetPosition(vec3 new_position) {
		position = new_position;
		viewMatrixDirty = true;
	}

	const vec3 GetPosition() const {
		return position;
	}

	void SetFOV(const float new_fov) {
		fov = new_fov;
		UpdateProjectionMatrix();
	}

	const float GetFOV() const {
		return fov;
	}

	void SetZoom(const float new_zoom) {
		zoom = new_zoom;
		UpdateProjectionMatrix();
	}

	const float GetZoom() {
		return zoom;
	}

	void SetViewMode(ViewMode view_mode) {
		viewMode = view_mode;
		UpdateProjectionMatrix();
	}

	const ViewMode GetViewMode() const {
		return viewMode;
	}

	void SetClippingPlane(float z_near, float z_far) {
		zNear = z_near;
		zFar = z_far;
		UpdateProjectionMatrix();
	}

	void SetSize(int new_width, int new_height) {
		width = new_width;
		height = new_height;
		UpdateProjectionMatrix();
	}

	const mat4* GetViewMatrix() {
		if(viewMatrixDirty) this->Update();
		return &viewMat;
	}

	const mat4* GetProjectionMatrix() const {
		return &projectionMatrix;
	}

	const vec3 ScreenToWorldCoordinates(int x_pos, int y_pos) {
		mat4 inverseMat = inverse(projectionMatrix*viewMat);

		float normalizedX = static_cast<float>(x_pos) / width;
		float normalizedY = static_cast<float>(y_pos) / height;
		//set range from -1 to 1;

		float screenNormalizedX = normalizedX * 2 - 1;
		float screenNormalizedY = normalizedY * 2 - 1;

		auto screenPos = vec4(screenNormalizedX, screenNormalizedY, position.z, 1);
		vec4 pos = inverseMat * screenPos;
		
		pos.w = 1.0f / pos.w;
		pos.x *= pos.w;
		pos.y *= pos.w;
		pos.z *= pos.w;

		return vec3(pos.x, pos.y, pos.z);
	}

	const vec3 ScreenToWorldCoordinatesDepth(int x_pos, int y_pos, int z_pos) {
		mat inverseMat = inverse(projectionMatrix);

		auto temp = vec4(x_pos, y_pos, z_pos, 1);
		int viewport[] = {0, 0, width, height};
		temp.x = (temp.x - viewport[0]) / viewport[2];
		temp.y = (temp.y - viewport[1]) / viewport[3];
		temp.z = (temp.z - zNear) / zFar; // normalize?
		temp = 2.0f * temp - temp;

		vec4 obj = inverseMat * temp;
		obj /= obj.w;

		return obj;
		//mat4 inverseMat = inverse(projectionMatrix);

		//float normalizedX = static_cast<float>(x_pos) / width;
		//float normalizedY = static_cast<float>(y_pos) / height;
		////set range from -1 to 1;

		//float screenNormalizedX = normalizedX * 2 - 1;
		//float screenNormalizedY = normalizedY * 2 - 1;

		//auto screenPos = vec4(screenNormalizedX, screenNormalizedY, z_pos, 1);
		//auto pos = inverseMat * screenPos;

		//std::cout << "x:" << pos.x << " y:" << pos.y << " z:" << pos.z << " w: " << pos.w << std::endl;

		//pos.w = 1.0f / pos.w;
		//pos.x *= pos.w;
		//pos.y *= pos.w;
		//pos.z *= pos.w;

		//return vec3(pos.x, pos.y, pos.z);
	}

	Camera(int width, int height, bool setMain = false) : width(width), height(height) {
		UpdateProjectionMatrix();

		inputBindings.push_back(Input::AddKeyBinding(SDLK_w, [this](KeyEvent e) {this->Move(this->Forward); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_s, [this](KeyEvent e) {this->Move(-this->Forward); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_a, [this](KeyEvent e) {this->Move(-this->Right); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_d, [this](KeyEvent e) {this->Move(this->Right); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_SPACE, [this](KeyEvent e){this->Move(this->Up); }));
		inputBindings.push_back(Input::AddKeyBinding(SDLK_x, [this](KeyEvent e) {this->Move(-this->Up); }));
		mouseBinding = Input::AddMouseBinding([this](const InputMouseEvent* e)
		{
			if (e->GetMouseKeyDown(MouseButton::Right)) {
				Input::SetMouseCapture(true);
				relativeMouseModeActive = true;
			}

			if (e->GetMouseKeyUp(MouseButton::Right)) {
				Input::SetMouseCapture(false);
				relativeMouseModeActive = false;
			}

			if (e->GetMouseKeyHold(MouseButton::Right)) {
				this->Rotate(e->motion->deltaX, e->motion->deltaY);
			}
		});

		if(setMain) {
			Main = this;
		}
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


