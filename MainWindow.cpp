#include "MainWindow.h"

#include <SDL.h>
#include <string>
#include <iostream>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>


#include "Files.h"

#include "Camera.h"
#include "GridToolBar.h"
#include "Renderer.h"
#include "Resources.h"
#include "Texture.h"
#include "TileMap.h"
#include "Tile.h"

using namespace Rendering;

MainWindow::MainWindow(const int new_width, const int new_height, const char* title) : m_title(title) {
	width = new_width;
	height = new_height;
}

int MainWindow::width = 0;
int MainWindow::height = 0;





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

void MainWindow::OnMouseInput(const InputMouseEvent* event) {
	gridToolBar->OnMouseEvent(event);
}

bool MainWindow::Initialize() {
	if (!InitSDL()) return false;
	if (!Renderer::Init()) return false;
	if (!InitDearImGui()) return false;

	tileMap = new Tiles::TileMap();
	tileMap->shader = Renderer::defaultShader;
	gridToolBar = new GridTools::GridToolBar(tileMap);

	Renderer::RenderObjects.push_back(tileMap);
	//update while resizing - does not work though, according to google its a backend limitation?
	SDL_AddEventWatch(WindowResizeEvent, this);
	binding = Input::AddMouseBinding([this](const InputMouseEvent* e) {this->OnMouseInput(e); });


	// Load Sprites from Sprites Folder to Memory
	if (Files::VerifyDirectory("Sprites")) {
		for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().append("Sprites"))) {
			std::string item = entry.path().string();

			if (!Files::IsSupportedImageFormat(item.c_str())) continue;

			Resources::LoadTexture(item);
		}
	}

	// Load Tiles from Tiles Folder to Memory
	if (Files::VerifyDirectory("Tiles")) {
		for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().append("Tiles"))) {
			std::string item = entry.path().string();
			Resources::LoadTile(item);
		}
	}

	return true;
}

bool MainWindow::InitSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL
		| SDL_WINDOW_RESIZABLE
		| SDL_WINDOW_ALLOW_HIGHDPI);

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
				std::string path;
				constexpr char filter[] = "Image Files (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM)\0*.jpeg;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic;*.pnm\0\0";
				if (Files::OpenFileDialog(path, filter)) {
					// success!
					std::cout << path << std::endl;
				}
			}
			ImGui::EndMenu();
		}

		if (MenuItem("Recompile Shader")) {
			Renderer::CompileShader();
		}

		static bool showTileCreationWindow = false;
		if (BeginMenu("Tiles")) {
			if (Files::VerifyDirectory("Tiles")) {
				if (ImGui::MenuItem("New Tile")) {
					showTileCreationWindow = true;
				}

				for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().append("Tiles"))) {
					std::string item = entry.path().string();
					if (ImGui::MenuItem(item.c_str())) {
					}
				}
			}

			ImGui::EndMenu();
		}

		if (showTileCreationWindow) {
			Tiles::Tile* t = nullptr;
			if (Tiles::Tile::ImGuiCreateTile(showTileCreationWindow, t)) {
				showTileCreationWindow = false;
				//tile created!
				//save file to disk
				//TODO: secure load/save function
				std::string path = "Tiles/" + t->Name;
				Files::SaveToFile(path.c_str(), t);
				std::cout << "Saved." << std::endl;
				auto loaded = Files::LoadFromFile<Tiles::Tile>(path.c_str());

				std::cout << "Loaded:" << std::endl;
				std::cout << loaded->Name << std::endl;
				std::cout << loaded->Texture << std::endl;
			}
		}

		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Show Demo Window", 0, showDebugWindow)) {
				showDebugWindow = !showDebugWindow;
			}
			if (ImGui::BeginMenu("Sprites")) {
				if (Files::VerifyDirectory("Sprites")) {
					for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path().append("Sprites"))) {
						std::string item = entry.path().string();
						if (!Files::IsSupportedImageFormat(item.c_str())) continue;

						if (ImGui::MenuItem(item.c_str())) {
							std::cout << "Item clicked: " << item << std::endl;
							Resources::LoadTexture(item, true);
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

	// Main Cam Controls
	Camera::Main->DearImGuiWindow();

	bool mouseOpen = true;
	auto mousePos = Input::GetMousePosition();
	auto mouseCoords = Camera::Main->ScreenToGridPosition(mousePos.x, mousePos.y);
	auto gridCoords = floor(mouseCoords);
	constexpr ImGuiWindowFlags mouseCoordFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize;
	if (ImGui::Begin("MouseCoordinates", &mouseOpen, mouseCoordFlags)) {
		ImGui::Text("Mouse/Grid Debug");
		if (ImGui::BeginTable("table_RawMouseCoords", 2)) {
			ImGui::TableNextRow();
			TableSetColumnIndex(0);
			Text(std::string("X: " + std::to_string(mouseCoords.x)).c_str());
			TableSetColumnIndex(1);
			Text(std::string("Y: " + std::to_string(mouseCoords.y)).c_str());
		}
		EndTable();
		ImGui::Text("Grid Coords:");
		if (ImGui::BeginTable("table_MouseGridCoords", 2)) {
			ImGui::TableNextRow();
			TableSetColumnIndex(0);
			Text(std::string("X: " + std::to_string((int)gridCoords.x)).c_str());
			TableSetColumnIndex(1);
			Text(std::string("Y: " + std::to_string((int)gridCoords.y)).c_str());
		}
		EndTable();

		const auto size = GetWindowSize();
	}
	ImGui::End();

	// Grid Tool Window
	bool toolWindowOpen = true;
	constexpr ImGuiWindowFlags toolFlags = ImGuiWindowFlags_AlwaysAutoResize;
	if (Begin("Tools", &toolWindowOpen, toolFlags)) {
		int toolCount = 3;
		auto buttonSize = ImVec2(32, 32);
		for (int i = 0; i < toolCount; ++i) {
			SameLine();
			std::string buttonName = "ToolButton" + std::to_string(i);
			ImGui::PushID(buttonName.c_str());
			bool selected = i == static_cast<int>(gridToolBar->GetActiveTool());
			int framePadding = selected ? 2 : 0;
			if (ImageButton((void*)0, buttonSize, ImVec2(0,0), ImVec2(1, 1), framePadding)) {
				auto toolType = static_cast<GridTools::GridToolType>(i);
				std::cout << "Selected: " << i << std::endl;
				gridToolBar->SelectTool(toolType);
			}
			PopID();
		}

	}
	End();

	constexpr ImGuiWindowFlags tilesFlags = ImGuiWindowFlags_AlwaysAutoResize;
	bool fexOpen = true;
	if (Begin("Tiles", &fexOpen, tilesFlags)) {
		//loaded images
		for (auto tileIterator = Resources::Tiles.begin(); tileIterator != Resources::Tiles.end(); ++tileIterator) {
			auto& tex = tileIterator->second->Texture;
			if (Texture* t; Resources::TryGetTexture(tex, t)) {
				const auto id = t->GetTextureID();
				bool isSelected = gridToolBar->GetSelectedTile() == tileIterator->second;
				int framePadding = isSelected ? 4 : 0;
				// Push and pop id required as ImageButton gets identified by texture ID which might be same as other tiles
				PushID(tileIterator->first.c_str()); 
				if (ImageButton((void*)id, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0), framePadding)) {
					gridToolBar->SetSelectedTile(tileIterator->second);
				}
				PopID();
				SameLine();
			}

		}
	}
	End();
	// Required to render ImGuI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void MainWindow::OnResized(int width, int height) {
	MainWindow::height = height;
	MainWindow::width = width;
	glViewport(0, 0, width, height);
	if (Camera::Main != nullptr) {
		Camera::Main->SetSize(width, height);
	}
}
void MainWindow::Close() {
	Input::RemoveMouseBinding(binding);
	binding = nullptr;
	delete tileMap;
	Renderer::Exit();
	SDL_DestroyWindow(SDLWindow);
	SDLWindow = nullptr;
}
