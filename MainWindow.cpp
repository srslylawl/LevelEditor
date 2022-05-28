#include <SDL.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

#include "MainWindow.h"

#include "Files.h"
#include "Shader.h"
#include "Time.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Renderer.h"
#include "Resources.h"

using namespace Rendering;

MainWindow::MainWindow(int width, int height, const char* title) : m_title(title) {
	this->width = width;
	this->height = height;
}

int MainWindow::width = 0;
int MainWindow::height = 0;

void TextCentered(const char* text) {
	float win_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text).x;

	// calculate the indentation that centers the text on one line, relative
	// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
	float text_indentation = (win_width - text_width) * 0.5f;

	// if text is too long to be drawn on one line, `text_indentation` can
	// become too small or even negative, so we check a minimum indentation
	float min_indentation = 20.0f;
	if (text_indentation <= min_indentation) {
		text_indentation = min_indentation;
	}

	ImGui::SameLine(text_indentation);
	ImGui::PushTextWrapPos(win_width - text_indentation);
	ImGui::TextWrapped(text);
	ImGui::PopTextWrapPos();
}

int WindowResizeEvent(void* data, SDL_Event* event) {
	if (event->type != SDL_WINDOWEVENT
		|| event->window.event != SDL_WINDOWEVENT_RESIZED) return -1;

	auto win = (MainWindow*)data;

	if (SDL_GetWindowFromID(event->window.windowID) != win->GetSDLWindow()) return -1;

	const int w = event->window.data1;
	const int h = event->window.data2;
	win->OnResized(w, h);

	return 0;
}

bool MainWindow::Initialize() {
	if (!InitSDL()) return false;
	if (!Renderer::Init(this)) return false;
	if (!InitDearImGui()) return false;

	//update while resizing - does not work though, according to google its a backend limitation?
	SDL_AddEventWatch(WindowResizeEvent, this);
	return true;
}

bool MainWindow::InitSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(
		SDL_WINDOW_OPENGL
		| SDL_WINDOW_RESIZABLE
		| SDL_WINDOW_ALLOW_HIGHDPI
		);

	SDLWindow = SDL_CreateWindow(m_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
	if (SDLWindow == nullptr) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(SDLWindow);
		return false;
	}

	return true;
}
bool MainWindow::InitDearImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// does not seem to do anything rn, so commented out
	//ImGuiIO& io = ImGui::GetIO();
	//(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(SDLWindow, Renderer::gl_context); //Renderer needs to be initialized first
	const std::string glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

	return true;
}
void MainWindow::Render() {
	// clear color, depth and stencil buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Order of Render OpenGL and ImGui does not seem to matter?
	Renderer::Render();
	RenderImGui();

	SDL_GL_SwapWindow(SDLWindow);
}

void MainWindow::RenderImGui() {
	// Required before ImGui logic
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(SDLWindow);
	using namespace ImGui;
	ImGui::NewFrame();

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	// ImGui logic here

	if (showDebugWindow) {
		ImGui::ShowDemoWindow();
	}

	// Menu Bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				//Do something
			}
			ImGui::EndMenu();
		}

		if(MenuItem("Recompile Shader")) {
			Renderer::CompileShader();
		}

		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Show Demo Window", 0, showDebugWindow)) {
				showDebugWindow = !showDebugWindow;
			}
			if (ImGui::BeginMenu("Sprites")) {
				if (Files::VerifyDirectory("Sprites")) {
					for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().append("Sprites"))) {
						std::string item = entry.path().string();
						if (ImGui::MenuItem(item.c_str())) {
							std::cout << "Item clicked: " << item << std::endl;
							Resources::LoadTexture(item);
						}
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::BeginMenu("Render mode")) {
				bool selected = renderMode == 0;
				if (ImGui::MenuItem("Default", 0, selected) && !selected) {
					// set rendermode to default
					renderMode = 0;
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				selected = renderMode == 1;
				if (ImGui::MenuItem("Wire-frame", 0, selected) && !selected) {
					// set rendermode to wireframe
					renderMode = 1;
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Camera Window -> TODO: should be transferred to the camera class
	constexpr ImGuiWindowFlags camFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
	bool open = true;
	if (ImGui::Begin("Camera", &open, camFlags)) {
		TextCentered("Camera");
		bool twoDEnabled = Camera::Main->GetDimensionMode() == DimensionMode::TwoDimensional;

		// 2D Checkbox
		if (ImGui::Checkbox("2D", &twoDEnabled)) {
			Camera::Main->SetDimensionMode(static_cast<DimensionMode>(twoDEnabled));
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
			vec3 pos = Camera::Main->GetPosition();
			for (int column = 0; column < columnCount; column++) {
				ImGui::TableSetColumnIndex(column);
				ImGui::AlignTextToFramePadding();
				Text(columns[column]);
				SameLine();
				// allow empty label by pushing ID and inputting "##" as label name
				PushID(column);
				PushItemWidth(-FLT_MIN);
				if (DragFloat("##", &pos[column], 0.05f, 0, 0, "%.6f")) {
					Camera::Main->SetPosition(pos);
				}
				PopItemWidth();
				PopID();
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
				vec3 rotation = Camera::Main->GetRotation();
				for (int column = 0; column < 3; column++) {
					ImGui::TableSetColumnIndex(column);
					ImGui::AlignTextToFramePadding();
					Text(columns[column]);
					SameLine();
					// allow empty label by pushing ID and inputting "##" as label name
					PushID(column);
					PushItemWidth(-FLT_MIN);
					if (DragFloat("##", &rotation[column], 0.05f, 0, 0, "%.6f")) {
						Camera::Main->SetRotation(rotation);
					}
					PopItemWidth();
					PopID();
				}
				ImGui::EndTable();
			}
			// Movespeed Slider
			ImGui::SliderFloat("Speed", &Camera::MoveSpeed, 0, 200);

			// Rotation Speed Slider
			ImGui::SliderFloat("RotationSpeed", &Camera::TurnSpeed, 0, 200);

			// FOV Slider
			float camFOV = Camera::Main->GetFOV();
			if (ImGui::SliderFloat("FOV", &camFOV, 0, 180, "%.0f")) {
				Camera::Main->SetFOV(camFOV);
			}

			// Zoom
			float zoom = Camera::Main->GetZoom();
			constexpr ImGuiSliderFlags zoomFlags = ImGuiSliderFlags_Logarithmic;
			constexpr float minZoom = 1 / 100.0f;
			if (ImGui::SliderFloat("Zoom", &zoom, minZoom, 50.0f, "%.6f", zoomFlags)) {
				Camera::Main->SetZoom(zoom);
			}

			// ViewMode (perspective)
			const char* items[] = { "Perspective", "Orthographic" };
			int current = static_cast<int>(Camera::Main->GetViewMode());
			constexpr int itemCount = static_cast<int>(std::size(items));
			if (ImGui::Combo("ViewMode", &current, items, itemCount)) {
				Camera::Main->SetViewMode(static_cast<ViewMode>(current));
			}
		}
		else {
			// 2D mode active
			float orthoSize = Camera::Main->GetOrthoSize();
			if (ImGui::SliderFloat("Size", &orthoSize, 1, 100, "%.5f")) {
				Camera::Main->SetOrthoSize(orthoSize);
			}
		}


		const auto size = GetWindowSize();
		ImGui::SetWindowPos(ImVec2(main_viewport->Size.x - size.x, main_viewport->Size.y - size.y));
	}
	ImGui::End();

	bool mouseOpen = true;

	auto mousePos = Input::GetMousePosition();
	//auto mouseCoords = Camera::Main->ScreenToWorldCoordinates(mousePos.x, mousePos.y);
	auto mouseCoords = Camera::Main->ScreenToGridPosition(mousePos.x, mousePos.y);
	constexpr ImGuiWindowFlags mouseCoordFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("MouseCoordinates", &mouseOpen)) {
		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame;
		if (ImGui::BeginTable("table_MouseCoords", 3, tableFlags)) {
			ImGui::TableNextRow();
			TableSetColumnIndex(0);
			Text(std::string("X: " + std::to_string(mouseCoords.x)).c_str());
			TableSetColumnIndex(1);
			Text(std::string("Y: " + std::to_string(mouseCoords.y)).c_str());
			TableSetColumnIndex(2);
			/// SET TO ZERO
			Text(std::string("Z: " + std::to_string(0)).c_str());
		}
		Text("ScreenPos:");
		ImGui::EndTable();
		if (ImGui::BeginTable("table_MouseCoordsScreen", 2, tableFlags)) {
			TableNextRow();
			TableSetColumnIndex(0);
			Text(std::string("X: " + std::to_string(mousePos.x)).c_str());
			TableSetColumnIndex(1);
			Text(std::string("X: " + std::to_string(mousePos.y)).c_str());
		}
		EndTable();

	}
	ImGui::End();

	constexpr ImGuiWindowFlags explorerFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
	bool fexOpen = true;
	if (Begin("File Explorer", &fexOpen, explorerFlags)) {
		//loaded images
		for (auto it = Resources::Textures.begin(); it != Resources::Textures.end(); ++it) {
			const auto& tex = it->second;
			Image((void*)tex.ID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0));
		}
	}
	End();
	// Required to render ImGuI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void MainWindow::OnResized(int width, int height) {
	this->height = height;
	this->width = width;
	glViewport(0, 0, width, height);
	if (Camera::Main != nullptr) {
		Camera::Main->SetSize(width, height);
	}
	//std::cout << "On Resized h: " << height << " w: " << width << endl;
}
void MainWindow::Close() {
	Renderer::Exit();
	SDL_DestroyWindow(SDLWindow);
	SDLWindow = nullptr;
}
