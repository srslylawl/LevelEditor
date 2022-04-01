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

using namespace std;

MainWindow::MainWindow(int width, int height, const char* title) : m_width(width), m_height(height), m_title(title) {}

bool MainWindow::Initialize() {
	InitSDL();
	InitOpenGL();
	InitDearImGui();

	//update while resizing - does not work though, according to google its a backend limitation?
	SDL_EventFilter filter = [](void* data, SDL_Event* event) -> int {
		if (!(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)) return 0;

		auto win = (MainWindow*)data;
		if (SDL_GetWindowFromID(event->window.windowID) == win->GetSDLWindow()) {
			const int w = event->window.data1;
			const int h = event->window.data2;
			win->OnResized(w, h);
		}

		return 0;
	};
	SDL_AddEventWatch(filter, this);
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
	}
	else {
		std::cout << "[INFO] glad initialized\n";
	}

	// __vertex input__
	// create vertex array object (VAO) and bind it
	glGenVertexArrays(1, &VertexArrayObject);
	glBindVertexArray(VertexArrayObject);

	// create vertex buffer object (VBO) and bind it, can only generate one of each type
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// create element buffer object (EBO) and bind it -> allows reusing of verts, binds it to VAO as well
	glGenBuffers(1, &elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);

	const float triangleVerts[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	const float quadVerts[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
	};

	const unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
	};

	//assign buffer data to EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//feed verts into array buffer -- since only one buffer was created, it picks that one
	// GL_STATIC_DRAW since we never change the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	// specify how to interpret vertex data
	// 0 = layout, specified in vert shader
	// 3 = size of vertex attribute, 3 since its a vector3
	// GL_FLOAT = datatype
	// GL_FALSE = dont normalize (used for booleans)
	// 3* size of float is STRIDE, aka space between vertex attributes. since its just packed tightly, exactly size of attribute
	// (void*)0 is a nullptr?
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// enable VAO
	// 0 = index
	glEnableVertexAttribArray(0);

	//init shaders
	const filesystem::path vertShaderPath = filesystem::current_path().append("Shaders/defaultVertShader.vert");
	const filesystem::path fragShaderPath = filesystem::current_path().append("Shaders/defaultFragShader.frag");

	shaderProgramUPTR = make_unique<Shader>(vertShaderPath.string().c_str(), fragShaderPath.string().c_str());

	return true;
}

bool MainWindow::RenderOpenGL() {
	//___ LOOPED RENDERING CODE
	// use shader program
	shaderProgramUPTR->use();

	// use VAO
	// EBO is already bound to VAO so it gets bound automatically
	glBindVertexArray(VertexArrayObject);

	// draw triangles using EBO, takes from bound element array buffer
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//unbind vertex array
	glBindVertexArray(0);


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

	SDLWindow = SDL_CreateWindow(m_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, window_flags);
	if (SDLWindow == nullptr) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(SDLWindow);
		return false;
	}

	std::cout << "\nOpening Window: " << m_title;

	return false;
}

bool MainWindow::InitDearImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(SDLWindow, gl_context);
	const std::string glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

	return true;
}

void MainWindow::OnResized(int width, int height) {
	m_height = height;
	m_width = width;
	glViewport(0, 0, m_width, m_height);
	std::cout << "On Resized h: " << height << " w: " << width << endl;
}

void MainWindow::RenderImGui() {
	/*ImGui::ShowDemoWindow();*/

	//return;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				//Do something
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("folderstuff")) {
				std::cout << "Items in Sprites: " << endl;
				if (VerifyDirectory("Sprites")) {
					for (const auto& entry : filesystem::directory_iterator(filesystem::current_path().append("Sprites")))
						std::cout << entry.path() << std::endl;
				}
			}
			if (ImGui::BeginMenu("Sprites")) {
				if (VerifyDirectory("Sprites")) {
					for (const auto& entry : filesystem::directory_iterator(filesystem::current_path().append("Sprites"))) {
						string item = entry.path().string();
						if (ImGui::MenuItem(item.c_str())) {
							cout << "Item clicked: " << item << endl;
						}
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Rendering")) {
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
}

void MainWindow::Render() {
	// set clearing color, clear gl buffers
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	//start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(MainWindow::SDLWindow);
	ImGui::NewFrame();

	//all ImGui UI stuff goes here
	MainWindow::RenderImGui();

	//render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	RenderOpenGL();

	SDL_GL_SwapWindow(MainWindow::SDLWindow);
}

void MainWindow::Close() {
	cout << "Closing window: " << m_title << endl;
	SDL_DestroyWindow(SDLWindow);
	SDLWindow = nullptr;
}
