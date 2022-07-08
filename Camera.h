#pragma once

#include "Input.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "MathExt.h"
#include "Time.h"
#include "imgui.h"
#include "ImGuiHelper.h"

namespace Rendering {
	using namespace glm;

	enum class ViewMode {
		Perspective = 0,
		Orthographic = 1
	};

	enum class DimensionMode {
		ThreeDimensional = 0,
		TwoDimensional = 1
	};

	class Camera {
		std::vector<InputKeyBinding*> inputBindings;
		InputMouseBinding* mouseBinding = nullptr;
		bool relativeMouseModeActive = false;

		bool viewMatrixDirty = true;

		mat4 projectionMatrix = {};
		mat4 viewMat = {};

		ViewMode viewMode;
		DimensionMode dimensionMode;

		int width;
		int height;
		float aspectRatio;
		float zNear = 0.0001f; // HAS to be >0
		float zFar = 200.0f;
		float fov = 45.0f;

		float zoom3D = 1;
		float zoom2D = 3;
		float orthoSize; // pixel perfect = resolution height divided by 2x pixel per unit size

		void UpdateProjectionMatrix() {
			if (viewMode == ViewMode::Perspective) {
				aspectRatio = width / static_cast<float>(height);
				projectionMatrix = perspectiveLH(radians(fov / zoom3D), aspectRatio, zNear, zFar);
				return;
			}

			if (viewMode == ViewMode::Orthographic) {
				const float zoomedOrthoSize = orthoSize/zoom2D;
				aspectRatio = width / static_cast<float>(height);
				const float halfW = zoomedOrthoSize * aspectRatio;
				const float halfH = zoomedOrthoSize;
				projectionMatrix = orthoLH(-halfW, halfW, -halfH, halfH, zNear, zFar);
				return;
			}
		}

		void UpdateForwardAxis() {
			Forward.x = cos(radians(rotation.y)) * cos(radians(rotation.x));
			Forward.y = sin(radians(rotation.x));
			Forward.z = sin(radians(rotation.y)) * cos(radians(rotation.x));

			Forward = -normalize(Forward);

			viewMatrixDirty = true;
		}

		void HandleMoveInput(SDL_KeyCode keyCode) {
			switch (dimensionMode) {
			case DimensionMode::ThreeDimensional:
				HandleMoveInput3D(keyCode);
				break;
			case DimensionMode::TwoDimensional:
				HandleMoveInput2D(keyCode);
				break;
			}
		}
		void HandleMoveInput2D(SDL_KeyCode keyCode) {
			switch (keyCode) {
			case SDLK_w:
				Move(Up);
				break;
			case SDLK_s:
				Move(-Up);
				break;
			case SDLK_a:
				Move(-Right);
				break;
			case SDLK_d:
				Move(Right);
				break;
			}
		}
		void HandleMoveInput3D(SDL_KeyCode keyCode) {
			switch (keyCode) {
			case SDLK_w:
				Move(Forward);
				break;
			case SDLK_s:
				Move(-Forward);
				break;
			case SDLK_a:
				Move(-Right);
				break;
			case SDLK_d:
				Move(Right);
				break;
			case SDLK_SPACE:
				Move(Up);
				break;
			case SDLK_x:
				Move(-Up);
				break;
			}
		}
		void HandleMouseInput(const InputMouseEvent* e) {
			switch (dimensionMode) {
			case DimensionMode::ThreeDimensional:
				HandleMouseInput3D(e);
				break;
			case DimensionMode::TwoDimensional:
				HandleMouseInput2D(e);
				break;
			}
		}
		void HandleMouseInput2D(const InputMouseEvent* e) {
			if (e->GetMouseKeyHold(MouseButton::Right)) {
				const vec2 screenPosDelta = vec2(-e->motion->deltaX, e->motion->deltaY);
				const vec2 currentScreenPos = vec2(e->motion->posX, e->motion->posY);
				const vec2 oldScreenPos = currentScreenPos - screenPosDelta;


				const vec2 oldGridPos = ScreenToGridPosition(oldScreenPos.x, oldScreenPos.y);
				const vec2 currGridPos = ScreenToGridPosition(currentScreenPos.x, currentScreenPos.y);
				const vec2 deltaGridPos = currGridPos - oldGridPos;
				SetPosition(position + vec3(deltaGridPos.x, deltaGridPos.y, 0));
			}

			const auto wheelDelta = e->GetMouseWheelDelta();
			if (wheelDelta != 0) {
				float newZoom = zoom2D;
				if(wheelDelta > 0) {
					if(zoom2D >= 1) newZoom++;
					else newZoom*=1.25f;
				}

				if(wheelDelta < 0) {
					if(zoom2D >= 2) newZoom--;
					else newZoom /= 1.25f;
				}

				if(newZoom > 1) {
					newZoom = round(newZoom);
				}

				SetZoom2D(newZoom);
			}
		}
		void HandleMouseInput3D(const InputMouseEvent* e) {
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

			const auto wheelDelta = e->GetMouseWheelDelta();
			if (wheelDelta != 0) {
				SetZoom3D(zoom3D + wheelDelta);
			}
		}
		vec3 position = vec3(0, 0, -3.0f); //in World Space
		vec3 rotation = vec3(0, -90, 0);
	public:
		inline static Camera* Main = nullptr;
		vec3 Forward = vec3(0, 0, 1.0f);
		vec3 Right{}; //Relative to Camera
		vec3 Up{}; //Relative to Camera

		inline static float MoveSpeed = 50.0f;
		inline static float TurnSpeed = 50.0f;

		~Camera() {
			for (auto binding : inputBindings) {
				Input::RemoveKeyBinding(binding);
			}
			Input::RemoveMouseBinding(mouseBinding);

			if (relativeMouseModeActive) {
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
		}

		Camera(int width, int height, bool setMain = false) {
			SetSize(width, height);
			UpdateProjectionMatrix();

			inputBindings = {
				Input::AddKeyBinding(SDLK_w, [this](KeyEvent e) {this->HandleMoveInput(SDLK_w); }),
				Input::AddKeyBinding(SDLK_s, [this](KeyEvent e) {this->HandleMoveInput(SDLK_s); }),
				Input::AddKeyBinding(SDLK_a, [this](KeyEvent e) {this->HandleMoveInput(SDLK_a); }),
				Input::AddKeyBinding(SDLK_d, [this](KeyEvent e) {this->HandleMoveInput(SDLK_d); }),
				Input::AddKeyBinding(SDLK_SPACE, [this](KeyEvent e) {this->HandleMoveInput(SDLK_SPACE); }),
				Input::AddKeyBinding(SDLK_x, [this](KeyEvent e) {this->HandleMoveInput(SDLK_x); })
			};
			mouseBinding = Input::AddMouseBinding([this](const InputMouseEvent* e) {this->HandleMouseInput(e); });

			if (setMain) {
				Main = this;
			}
			SetDimensionMode(DimensionMode::TwoDimensional);
		}

		void Move(vec3 moveDirection) {
			position += moveDirection * MoveSpeed * Time::GetDeltaTime();

			viewMatrixDirty = true;
		}

		void Rotate(int deltaX, int deltaY) {
			const auto relative = TurnSpeed * Time::GetDeltaTime();
			rotation.y -= deltaX * relative;
			rotation.x += deltaY * relative;

			rotation.x = clamp(rotation.x, -89.0f, 89.0f);

			UpdateForwardAxis();
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

		void SetRotation(vec3 new_rotation) {
			rotation = new_rotation;
			UpdateForwardAxis();
		}

		void SetDimensionMode(DimensionMode dimension_mode) {
			switch (dimension_mode) {
			case DimensionMode::ThreeDimensional:
				SetViewMode(ViewMode::Perspective);
				break;
			case DimensionMode::TwoDimensional:
				SetViewMode(ViewMode::Orthographic);
				SetRotation({ 0, -90, 0 });
				SetPosition({ position.x, position.y, -5 });
				break;
			}

			dimensionMode = dimension_mode;
		}

		DimensionMode GetDimensionMode() const { return dimensionMode; }

		vec3 GetRotation() const {
			return rotation;
		}

		vec3 GetPosition() const {
			return position;
		}

		void SetFOV(const float new_fov) {
			fov = new_fov;
			UpdateProjectionMatrix();
		}

		float GetFOV() const {
			return fov;
		}

		void SetZoom3D(const float new_zoom) {
			zoom3D = max(new_zoom, 1.0f);
			UpdateProjectionMatrix();
		}

		float GetZoom3D() const {
			return zoom3D;
		}

		void SetZoom2D(const float new_zoom) {
			zoom2D = max(new_zoom, 0.00001f); //avoid divide by 0
			UpdateProjectionMatrix();
		}

		float GetZoom2D() const {
			return zoom2D;
		}

		void SetOrthoSize(const float new_size) {
			orthoSize = max(new_size, 0.0001f); //avoid divide by 0
			UpdateProjectionMatrix();
		}

		float GetOrthoSize() const { return orthoSize; }

		void SetViewMode(ViewMode view_mode) {
			viewMode = view_mode;
			UpdateProjectionMatrix();
		}

		ViewMode GetViewMode() const {
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
			orthoSize = height / 32.0f;
			UpdateProjectionMatrix();
		}

		const mat4* GetViewMatrix() {
			if (viewMatrixDirty) this->Update();
			return &viewMat;
		}

		const mat4* GetProjectionMatrix() const {
			return &projectionMatrix;
		}

		Ray ScreenPointToRay(int x_pos, int y_pos) const {
			// is NOT accurate in 3D!
			const mat inverseMat = inverse(projectionMatrix * viewMat);

			const auto normalizedX = x_pos / static_cast<float>(width);
			const auto normalizedY = y_pos / static_cast<float>(height);
			vec4 results[] = {
				vec4(normalizedX, normalizedY, -1, 1), //near
				vec4(normalizedX, normalizedY, 1, 1), }; //far

			for (auto& result : results) {
				//normalize to -1/+1 from 0/+1
				result = result * 2.0f - vec4(1.0f);
				result = inverseMat * result;
				result /= result.w;
			}

			return Ray(results[0], normalize(results[1] - results[0]));
		}

		vec2 ScreenToGridPosition(int x_pos, int y_pos) const {
			auto ray = ScreenPointToRay(x_pos, y_pos);

			//just doing plane collision up in here
			auto planePos = vec3(0);
			auto planeNormal = vec3(0, 0, -1);

			auto normalDot = dot(planeNormal, ray.Direction);
			// normalDot is positive when it hits the backface; negative when it hits front face

			if (abs(normalDot) < 1e-6) {
				//no collision, lines are parallel and have infinite collisions
				return vec2(0);
			}

			auto delta = planePos - ray.Origin;
			auto posOnRay = dot(delta, planeNormal) / normalDot;
			if (posOnRay < 0) {
				//no collision, ray never intersects
				return vec2(0);
			}
			// collision
			auto collisionPoint = ray.Origin + ray.Direction * posOnRay;
			return collisionPoint;
		}

		vec3 ScreenToWorldCoordinatesDepth(int x_pos, int y_pos, int z_pos) const {
			const mat inverseMat = inverse(projectionMatrix * viewMat);

			auto normalizedX = x_pos / static_cast<float>(width);
			auto normalizedY = y_pos / static_cast<float>(height);
			vec4 results[] = {
				vec4(normalizedX, normalizedY, -1, 1), //near
				vec4(normalizedX, normalizedY, 1, 1), }; //far

			for (auto& result : results) {
				//normalize to -1/+1 from 0/+1
				result = result * 2.0f - vec4(1.0f);
				result = inverseMat * result;
				result /= result.w;
			}

			auto dir = normalize(results[1] - results[0]);
			auto deltaZ = z_pos - position.z;

			return results[0] + (dir * deltaZ);
		}

		void DearImGuiWindow() {
			using namespace ImGui;
			constexpr ImGuiWindowFlags camFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
			bool open = true;
			if (ImGui::Begin("Camera", &open, camFlags)) {
				ImGuiHelper::TextCentered("Camera");
				bool twoDEnabled = GetDimensionMode() == DimensionMode::TwoDimensional;

				// 2D Checkbox
				if (ImGui::Checkbox("2D", &twoDEnabled)) {
					SetDimensionMode(static_cast<DimensionMode>(twoDEnabled));
				}

				// Transform Position
				const char* columns[] = { "X:", "Y:", "Z:" };
				const int columnCount = twoDEnabled ? 2 : 3;
				constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame;
				ImGui::Text("Transform Position");
				if (ImGui::BeginTable("table_Camera_Main_Transform", columnCount, tableFlags)) {
					ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_NoHeaderLabel;
					ImGui::TableSetupColumn("X", columnFlags);
					ImGui::TableSetupColumn("Y", columnFlags);
					if (!twoDEnabled) {
						ImGui::TableSetupColumn("Z", columnFlags);
					}
					ImGui::TableNextRow();
					glm::vec3 pos = GetPosition();
					for (int column = 0; column < columnCount; column++) {
						ImGui::TableSetColumnIndex(column);
						ImGui::AlignTextToFramePadding();
						Text(columns[column]);
						SameLine();
						// allow empty label by pushing ID and inputting "##" as label name
						PushID(column);
						PushItemWidth(-FLT_MIN);
						if (DragFloat("##", &pos[column], 0.05f, 0, 0, "%.6f")) {
							SetPosition(pos);
						}
						PopItemWidth();
						ImGui::PopID();
					}
					ImGui::EndTable();
				}

				if (!twoDEnabled) {
					// Rotation
					ImGui::Text("Transform Rotation");
					if (ImGui::BeginTable("table_Camera_Main_Rotation", 3, tableFlags)) {
						ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_NoHeaderLabel;
						ImGui::TableSetupColumn("X", columnFlags);
						ImGui::TableSetupColumn("Y", columnFlags);
						ImGui::TableSetupColumn("Z", columnFlags);
						ImGui::TableNextRow();
						glm::vec3 rotation = GetRotation();
						for (int column = 0; column < 3; column++) {
							ImGui::TableSetColumnIndex(column);
							ImGui::AlignTextToFramePadding();
							Text(columns[column]);
							SameLine();
							// allow empty label by pushing ID and inputting "##" as label name
							PushID(column);
							PushItemWidth(-FLT_MIN);
							if (DragFloat("##", &rotation[column], 0.05f, 0, 0, "%.6f")) {
								SetRotation(rotation);
							}
							PopItemWidth();
							ImGui::PopID();
						}
						ImGui::EndTable();
					}
					// Movespeed Slider
					ImGui::SliderFloat("Speed", &Camera::MoveSpeed, 0, 200);

					// Rotation Speed Slider
					ImGui::SliderFloat("RotationSpeed", &Camera::TurnSpeed, 0, 200);

					// FOV Slider
					float camFOV = GetFOV();
					if (ImGui::SliderFloat("FOV", &camFOV, 0, 180, "%.0f")) {
						SetFOV(camFOV);
					}

					// Zoom
					float zoom = GetZoom3D();
					constexpr ImGuiSliderFlags zoomFlags = ImGuiSliderFlags_Logarithmic;
					constexpr float minZoom = 1 / 100.0f;
					if (ImGui::SliderFloat("Zoom", &zoom, minZoom, 50.0f, "%.6f", zoomFlags)) {
						SetZoom3D(zoom);
					}

					// ViewMode (perspective)
					const char* items[] = { "Perspective", "Orthographic" };
					int current = static_cast<int>(GetViewMode());
					constexpr int itemCount = static_cast<int>(std::size(items));
					if (ImGui::Combo("ViewMode", &current, items, itemCount)) {
						SetViewMode(static_cast<ViewMode>(current));
					}
				}
				else {
					// 2D mode active
					float zoom_2d = GetZoom2D();
					constexpr float minZoom = 1.0f/25.0f;
					if (ImGui::SliderFloat("Size", &zoom_2d, minZoom, 25, "%.5f")) {
						SetZoom2D(zoom_2d);
					}
				}

				const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

				const auto size = GetWindowSize();
				ImGui::SetWindowPos(ImVec2(main_viewport->Size.x - size.x, main_viewport->Size.y - size.y));
			}
			ImGui::End();
		}

	};
}



