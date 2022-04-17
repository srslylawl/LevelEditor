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
#include "Resources.h"


MainWindow::MainWindow(int width, int height, const char* title) : m_width(width), m_height(height), m_title(title),
elementBufferObject(0), mainCamera(new Camera(m_width, m_height)) {}


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
	if (!InitOpenGL()) return false;
	if (!InitDearImGui()) return false;

	//update while resizing - does not work though, according to google its a backend limitation?
	SDL_AddEventWatch(WindowResizeEvent, this);
	return true;
}

static float ObjectOffsetX = 0;
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

	SDLWindow = SDL_CreateWindow(m_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, window_flags);
	if (SDLWindow == nullptr) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(SDLWindow);
		return false;
	}

	return true;
}
bool MainWindow::InitOpenGL() {
	// set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE
	);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

	gl_context = SDL_GL_CreateContext(SDLWindow);
	if (!gl_context) {
		std::cout << "Unable to create gl context " << SDL_GetError() << std::endl;
		return false;
	}
	SDL_GL_MakeCurrent(SDLWindow, gl_context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
		return false;
	}

	// set clearing color (background color)
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	// enable z testing
	glEnable(GL_DEPTH_TEST);

	float cubeVerts[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
	};
	float cubeTexCoords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f

	};
	meshes.emplace_back(cubeVerts, cubeTexCoords, 36);

	//init shaders
	const std::filesystem::path vertShaderPath = std::filesystem::current_path().append("Shaders/defaultVertShader.vert");
	const std::filesystem::path fragShaderPath = std::filesystem::current_path().append("Shaders/defaultFragShader.frag");

	shaderProgramUPTR = std::make_unique<Shader>(vertShaderPath.string().c_str(), fragShaderPath.string().c_str());

	return true;
}
bool MainWindow::InitDearImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// does not seem to do anything rn, so commented out
	//ImGuiIO& io = ImGui::GetIO();
	//(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(SDLWindow, gl_context);
	const std::string glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

	return true;
}
void MainWindow::Render() {
	// clear color, depth and stencil buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Order of Render OpenGL and ImGui does not seem to matter?
	RenderOpenGL();
	RenderImGui();

	SDL_GL_SwapWindow(SDLWindow);
}
void MainWindow::RenderOpenGL() {
	using namespace glm;
	//___ LOOPED RENDERING CODE
	// use shader program
	shaderProgramUPTR->use();
	//glUniform1i(glGetUniformLocation(shaderProgramUPTR->ID, "texture1"), 0); // -- redundant?

	// view matrix transforms world space to view (camera) space
	shaderProgramUPTR->setMat4("view", *mainCamera->GetViewMatrix());

	// projection matrix transforms view space to however we want to display (orthogonal, perspective)
	shaderProgramUPTR->setMat4("projection", *mainCamera->GetProjectionMatrix());

	glActiveTexture(GL_TEXTURE0);

	if(Resources::Textures.begin() != Resources::Textures.end()) {
		glBindTexture(GL_TEXTURE_2D, Resources::Textures.begin()->second.ID);
	}

	const vec3 cubePositions[] = {
	vec3(0.0f,  0.0f,  0.0f),
	vec3(2.0f,  2.0f, 2.0f),
	vec3(-2.0f, -2.0f, -2.0f),
	vec3(-3.8f, -2.0f, -12.3f),
	vec3(2.4f, -0.4f, -3.5f),
	vec3(-1.7f,  3.0f, -7.5f),
	vec3(1.3f, -2.0f, -2.5f),
	vec3(1.5f,  2.0f, -2.5f),
	vec3(1.5f,  0.2f, -1.5f),
	vec3(-1.3f,  1.0f, -1.5f)
	};

	for (int i = 0; i < 3; i++) {
		auto pos = cubePositions[i];
		pos.x += ObjectOffsetX;
		// model matrix transforms coords to world space
		mat4 modelM = translate(mat4(1.0f), pos);
		modelM = rotate(modelM, radians(90.0f * Time::GetTime() * (i + 1)), vec3(1.0f, 0.3f, 0.5f));
		shaderProgramUPTR->setMat4("model", modelM);

		/*glDrawArrays(GL_TRIANGLES, 0, 36);*/
		meshes[0].Draw();
	}

	//unbind vertex array
	glBindVertexArray(0);
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
	//return;

	// Menu Bar (Top of Window thing)
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				//Do something
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Show Demo Window", 0, showDebugWindow)) {
				showDebugWindow = !showDebugWindow;
			}
			if (ImGui::BeginMenu("ObjectOffsetX")) {
				DragFloat("ObjectOffsetX", &ObjectOffsetX);
				ImGui::EndMenu();
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

	// Camera Window -> should be transferred to the camera class
	constexpr ImGuiWindowFlags camFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
	bool open = true;
	if (ImGui::Begin("Camera", &open, camFlags)) {
		TextCentered("Camera");
		ImGui::SliderFloat("Speed", &Camera::MoveSpeed, 0, 200);
		ImGui::SliderFloat("RotationSpeed", &Camera::TurnSpeed, 0, 200);

		float camFOV = mainCamera->GetFOV();
		if (ImGui::SliderFloat("FOV", &camFOV, 0, 180, "%.0f")) {
			mainCamera->SetFOV(camFOV);
		}

		const char* columns[] = { "X:", "Y:", "Z:" };
		constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchSame;
		ImGui::Text("Transform");

		if (ImGui::BeginTable("table_Camera_Main_Transform", 3, tableFlags)) {
			ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_NoHeaderLabel;
			ImGui::TableSetupColumn("X", columnFlags);
			ImGui::TableSetupColumn("Y", columnFlags);
			ImGui::TableSetupColumn("Z", columnFlags);
			ImGui::TableNextRow();
			vec3 pos = mainCamera->GetPosition();
			for (int column = 0; column < 3; column++) {
				ImGui::TableSetColumnIndex(column);
				ImGui::AlignTextToFramePadding();
				Text(columns[column]);
				SameLine();
				// allow empty label by pushing ID and inputting "##" as label name
				PushID(column);
				PushItemWidth(-FLT_MIN);
				if(DragFloat("##", &pos[column], 0.05f, 0, 0, "%.6f")) {
					mainCamera->SetPosition(pos);
				}
				PopItemWidth();
				PopID();
			}
			ImGui::EndTable();
		}
		float zoom = mainCamera->GetZoom();
		constexpr ImGuiSliderFlags zoomFlags = ImGuiSliderFlags_Logarithmic;
		if (ImGui::SliderFloat("Zoom", &zoom, 0.5f, 150.0f, "%.1f", zoomFlags)) {
			mainCamera->SetZoom(zoom);
		}

		const char* items[] = { "Perspective", "Orthographic" };
		int current = static_cast<int>(mainCamera->GetViewMode());
		constexpr int itemCount = std::size(items);
		if (ImGui::Combo("ViewMode", &current, items, itemCount)) {
			mainCamera->SetViewMode(static_cast<ViewMode>(current));
		}
		const auto size = GetWindowSize();
		ImGui::SetWindowPos(ImVec2(main_viewport->Size.x - size.x, main_viewport->Size.y - size.y));
	}
	ImGui::End();

	constexpr ImGuiWindowFlags explorerFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
	bool fexOpen = true;
	if(Begin("File Explorer", &fexOpen, explorerFlags)) {
		//loaded images
		for(auto it = Resources::Textures.begin(); it != Resources::Textures.end(); ++it) {
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
	m_height = height;
	m_width = width;
	glViewport(0, 0, m_width, m_height);
	//std::cout << "On Resized h: " << height << " w: " << width << endl;
}
void MainWindow::Close() {
	SDL_DestroyWindow(SDLWindow);
	SDLWindow = nullptr;
	delete mainCamera;
}
