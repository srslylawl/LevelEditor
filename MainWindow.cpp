// ReSharper disable CppLocalVariableMayBeConst
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
#include "FileBrowser.h"
#include "GridToolBar.h"
#include "ImGuiHelper.h"
#include "Renderer.h"
#include "Resources.h"
#include "TextureSheet.h"
#include "Strings.h"
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






void MainWindow::OnMouseInput(const InputMouseEvent* event) {
	gridToolBar->OnMouseEvent(event);
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
	if (!Renderer::Init()) return false;
	if (!InitDearImGui()) return false;

	tileMap = new Tiles::TileMap();
	tileMap->shader = Renderer::defaultShader;
	gridToolBar = new GridTools::GridToolBar(tileMap);

	Renderer::RenderObjects.push_back(tileMap);
	//update while resizing - does not work though, according to google its a backend limitation?


	//SDL_AddEventWatch(WindowResizeEvent, this);
	binding = Input::AddMouseBinding([this](const InputMouseEvent* e) {this->OnMouseInput(e); });


	// Load Sprites and Tool Icons to Memory
	if (Files::VerifyDirectory(Strings::Directory_Sprites))
		Files::ForEachInDirectory(Strings::Directory_Sprites, [](const char* path) {Resources::LoadTexture(path); });

	if (Files::VerifyDirectory(Strings::Directory_Icon))
		Files::ForEachInDirectory(Strings::Directory_Icon, [](const char* path) {Resources::LoadInternalTexture(path); });

	// Load all textures in TextureSheets first
	if (Files::VerifyDirectory(Strings::Directory_TextureSheets)) {
		Resources::HandleTextureSheetFolder(false);
	}

	// Load Tiles from Tiles Folder to Memory
	constexpr char tileDir[] = "Tiles";
	if (Files::VerifyDirectory(tileDir))
		Files::ForEachInDirectory(tileDir, [](const char* path) {Resources::LoadTile(path); });


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

	static bool showDebugWindow = false;
	if (showDebugWindow) {
		ImGui::ShowDemoWindow();
	}

	static FileBrowser tileFileBrowser(Strings::Directory_Tiles, "Tiles",
									   [this](const FileBrowserFile& file) {
		if (file.FileType != FileBrowserFileType::Tile) return;
		const auto tile = static_cast<Tiles::Tile*>(file.Data);
		gridToolBar->SetSelectedTile(tile);
	}, false, [this](const FileBrowserFile& file) -> bool {
		if (file.FileType != FileBrowserFileType::Tile) return false;
		const auto tile = static_cast<Tiles::Tile*>(file.Data);
		return tile == gridToolBar->GetSelectedTile();
	});

	static FileBrowser spriteFileBrowser(Strings::Directory_Sprites, "Sprites", [](const FileBrowserFile& file) {
		if (file.FileType != FileBrowserFileType::Sprite) return;
		auto p = file.directory_entry.path().string();
		std::cout << "Pressed: " << p.c_str() << std::endl;
	});

	static TextureSheet* selectedTextureSheet = nullptr;

	constexpr char selectedTexSheetPopupID[] = "SelectedTextureSheet";
	static bool openTexSheetPopup = false;

	static FileBrowser textureSheetFileBrowser(Strings::Directory_TextureSheets, "TextureSheets", [](const FileBrowserFile& file) {
		if (file.FileType != FileBrowserFileType::TextureSheet) return;
		selectedTextureSheet = static_cast<TextureSheet*>(file.Data);
		openTexSheetPopup = true;
	}, true);



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

		static bool openSpriteSheetEditor = false;
		if (MenuItem("SpriteSheetEditor")) {
			openSpriteSheetEditor = true;
		}

		if (openSpriteSheetEditor) {
			if (Begin("TextureSheet Editor", &openSpriteSheetEditor, ImGuiWindowFlags_AlwaysAutoResize)) {
				static unsigned int texID = 0;
				static int offsetX = 0;
				static int offsetY = 0;
				static int width = 0;
				static int height = 0;
				static int channels = 0;
				static TextureSheet* sheet = nullptr;
				ImGuiHelper::Image(texID, ImVec2(128, 128));
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureSheet")) {
						sheet = *static_cast<TextureSheet**>(payload->Data);
						texID = sheet->GetMainTexture()->GetTextureID();
						width = sheet->GetMainTexture()->GetImageProperties().width;
						height = sheet->GetMainTexture()->GetImageProperties().height;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::InputInt("Offset X", &offsetX);
				ImGui::InputInt("Offset Y", &offsetY);
				ImGui::InputInt("Width", &width);
				ImGui::InputInt("Height", &height);

				static unsigned int newTextureId = 0;

				ImGuiHelper::Image(newTextureId, ImVec2(128, 128));
				if (sheet != nullptr && Button("Auto Slice")) {
					sheet->AutoSlice();
					spriteFileBrowser.RefreshCurrentDirectory();
				}

			}
			End();
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
						std::cout << "Abs: " << item << " Rel: " << Files::GetRelativePath(item) << std::endl;
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
				std::string path = "Tiles\\" + t->Name + Tiles::Tile::FileEnding;
				Files::SaveToFile(t);
				std::cout << "Saved." << std::endl;

				if (!Resources::LoadTile(path.c_str())) {
					std::cout << "Unable to load tile into resources!" << std::endl;
				}

				Tiles::Tile* loaded = nullptr;
				if (!Files::LoadFromFile(path.c_str(), loaded)) {
					std::cout << "Unabled to load created Tile:" << std::endl;
				}
				tileFileBrowser.RefreshCurrentDirectory();
			}
		}
		static bool showTextureDebugViewer = false;

		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Show Demo Window", 0, showDebugWindow)) {
				showDebugWindow = !showDebugWindow;
			}
			if (ImGui::MenuItem("Show TextureDebugViewer", nullptr, showTextureDebugViewer)) {
				showTextureDebugViewer = !showTextureDebugViewer;
			}
			ImGui::EndMenu();
		}

		if (showTextureDebugViewer) {
			if (ImGui::Begin("TextureDebugViewer", &showTextureDebugViewer, ImGuiWindowFlags_AlwaysAutoResize)) {
				static unsigned int textureId = 0;
				constexpr unsigned int step = 1;
				InputScalar("Texture ID", ImGuiDataType_U32, &textureId, &step);
				ImGuiHelper::Image(textureId, ImVec2(64, 64));
			}
			End();
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

	static bool mouseOpen = true;
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
	static bool toolWindowOpen = true;
	constexpr ImGuiWindowFlags toolFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
	if (Begin("Tools", &toolWindowOpen, toolFlags)) {
		int toolCount = 3;
		auto buttonSize = ImVec2(32, 32);
		const char* iconStrings[] = { Strings::Tool_Place, Strings::Tool_Erase, Strings::Tool_Select };
		for (int i = 0; i < toolCount; ++i) {
			std::string buttonName = "ToolButton" + std::to_string(i);
			ImGui::PushID(buttonName.c_str());
			bool isSelected = i == static_cast<int>(gridToolBar->GetActiveTool());

			if (isSelected) {
				PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
			}
			int framePadding = 2;
			int textureID = 0;
			if (Rendering::Texture* tex = nullptr; Resources::TryGetInternalTexture(iconStrings[i], tex))
				textureID = tex->GetTextureID();

			if (ImageButton((void*)textureID, buttonSize, ImVec2(0, 1), ImVec2(1, 0), framePadding)) {
				auto toolType = static_cast<GridTools::GridToolType>(i);
				std::cout << "Selected: " << i << std::endl;
				gridToolBar->SelectTool(toolType);
			}
			PopID();

			if (isSelected) PopStyleColor();
			SameLine();
		}

		float yPos = main_viewport->Size.y - main_viewport->WorkSize.y;
		ImGui::SetWindowPos(ImVec2(0, yPos));
	}
	End();


	tileFileBrowser.RenderRearImGuiWindow();
	spriteFileBrowser.RenderRearImGuiWindow();
	textureSheetFileBrowser.RenderRearImGuiWindow();

	if (selectedTextureSheet != nullptr) {
		if (openTexSheetPopup) {
			OpenPopup(selectedTexSheetPopupID);
			openTexSheetPopup = false;
		}

		if (BeginPopup(selectedTexSheetPopupID)) {
			selectedTextureSheet->RenderImGuiWindow();
			EndPopup();
		}
	}

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
