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

using namespace std;

MainWindow::MainWindow(int width, int height, const char* title) {
	m_width = width;
	m_height = height;
	m_title = title;
}

bool MainWindow::Initialize() {
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

	// set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE
	);

	std::string glsl_version = "";
#ifdef __APPLE__
	// GL 3.2 Core + GLSL 150
	glsl_version = "#version 150";
	SDL_GL_SetAttribute( // required on Mac OS
		SDL_GL_CONTEXT_FLAGS,
		SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
	);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
	// GL 3.2 Core + GLSL 150
	glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
	// GL 3.0 + GLSL 130
	glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

	SDL_GLContext gl_context = SDL_GL_CreateContext(SDLWindow);
	SDL_GL_MakeCurrent(SDLWindow, gl_context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
	}
	else {
		std::cout << "[INFO] glad initialized\n";
	}

	glViewport(0, 0, m_height, m_width);

	//update while resizing - does not work though, according to google its a backend limitation?
	SDL_EventFilter filter = [](void* data, SDL_Event* event) -> int {
		if (!(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED)) return 0;

		MainWindow* win = (MainWindow*)data;
		if (SDL_GetWindowFromID(event->window.windowID) == win->GetSDLWindow()) {
			int w = event->window.data1;
			int h = event->window.data2;
			win->OnResized(h, w);
		}

		return 0;
	};
	SDL_AddEventWatch(filter, this);

	//Setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(SDLWindow, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

	ImVec4 background = ImVec4(35 / 255.0f, 35 / 255.0f, 35 / 255.0f, 1.00f);
	glClearColor(background.x, background.y, background.z, background.w);
	return true;
}

void MainWindow::OnResized(int height, int width) {
	m_height = height;
	m_width = width;
	glViewport(0, 0, m_height, m_width);
}

void MainWindow::RenderImGui() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				//Do something
			}
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("folderstuff")) {
				std::cout << "Items in Sprites: " << endl;
				if(Files::VerifyDirectory("Sprites")) {
					for (const auto& entry : filesystem::directory_iterator(filesystem::current_path().append("Sprites")))
						std::cout << entry.path() << std::endl;
				}
			}
			if(ImGui::BeginMenu("Sprites")) {
				if (Files::VerifyDirectory("Sprites")) {
					for (const auto& entry : filesystem::directory_iterator(filesystem::current_path().append("Sprites"))) {
						string item = entry.path().string();
						if(ImGui::MenuItem(item.c_str())) {
							cout << "Item clicked: " << item << endl;
						}
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void MainWindow::Render() {
	// start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(MainWindow::SDLWindow);
	ImGui::NewFrame();

	//all ImGui UI stuff goes here
	MainWindow::RenderImGui();

	//render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(MainWindow::SDLWindow);
}

void MainWindow::Close() {
	std::cout << "\nClosing window: " << m_title;
	SDL_DestroyWindow(SDLWindow);
	SDLWindow = nullptr;
}
