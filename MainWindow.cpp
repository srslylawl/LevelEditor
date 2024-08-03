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


#include "AssetId.h"
#include "Files.h"

#include "Camera.h"
#include "FileBrowser.h"
#include "FileEditWindow.h"
#include "GridToolBar.h"
#include "ImGuiHelper.h"
#include "Renderer.h"
#include "Resources.h"
#include "TextureSheet.h"
#include "Strings.h"
#include "Texture.h"
#include "TileMap.h"
#include "TileMapManager.h"
#include "Tile.h"
#include "Level.h"
#include "DPIScale.h"

using namespace Rendering;

MainWindow::MainWindow(const int new_width, const int new_height, const char* title) : windowTitle(title) {
	width = new_width;
	height = new_height;

	SetWindowTitle();
}

int MainWindow::width = 0;
int MainWindow::height = 0;

void MainWindow::OnMouseInput(const InputMouseEvent* event) {
	bool success = gridToolBar->OnMouseEvent(event);
	if (success) SetWindowDirtyFlag(true);
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

	gridToolBar = new GridTools::GridToolBar();

	// update while resizing - does not work though, according to google its a backend limitation?
	//SDL_AddEventWatch(WindowResizeEvent, this);
	binding = Input::AddMouseBinding([this](const InputMouseEvent* e) {this->OnMouseInput(e); });

	Level* level = Level::CreateDefaultLevel();
	LoadLevel(level);

	// Load Resources
	Files::VerifyDirectory(Strings::Directory_Resources);

	if (Files::VerifyDirectory(Strings::Directory_Resources_Icons))
		Resources::LoadDirectory(Strings::Directory_Resources_Icons, false, true);
	if (Files::VerifyDirectory(Strings::Directory_Sprites))
		Resources::LoadDirectory(Strings::Directory_Sprites, false, true);
	if (Files::VerifyDirectory(Strings::Directory_TextureSheets))
		Resources::LoadDirectory(Strings::Directory_TextureSheets, false, true);
	if (Files::VerifyDirectory(Strings::Directory_Tiles))
		Resources::LoadDirectory(Strings::Directory_Tiles, false, true);


	auto onTileEdit = [](FileBrowserFile& file) {
		if (file.AssetHeader.aType == AssetType::Tile) {
			FileEditWindow::New(static_cast<Tiles::Tile*>(file.Data), [&file] {file.FileBrowser->RefreshCurrentDirectory(); });
		}
	};

	auto tileFileBrowser = new FileBrowser(Strings::Directory_Tiles, "Tiles",
										   [this](const FileBrowserFile& file) {
		if (file.AssetHeader.aType != AssetType::Tile) return;
		const auto tile = static_cast<Tiles::Tile*>(file.Data);
		gridToolBar->SetSelectedTile(tile);
	}, [this](const FileBrowserFile& file) -> bool {
		if (file.AssetHeader.aType != AssetType::Tile) return false;
		const auto tile = static_cast<Tiles::Tile*>(file.Data);
		return tile == gridToolBar->GetSelectedTile();
	}, onTileEdit, [](FileBrowser* browser) {
		FileCreationWindow::New<Tiles::Tile>(browser->GetCurrentDirectory(), [browser] {
			browser->RefreshCurrentDirectory();
		});
	});

	auto onTexSheetEdit = [](FileBrowserFile& file) {
		if (file.AssetHeader.aType == AssetType::TextureSheet) {
			FileEditWindow::New(static_cast<Rendering::TextureSheet*>(file.Data), [&file] {file.FileBrowser->RefreshCurrentDirectory(); });
		}
	};

	auto spriteFileBrowser = new FileBrowser(Strings::Directory_Sprites, "Sprites", [](const FileBrowserFile& file) {
		if (file.AssetHeader.aType != AssetType::Texture) return;
		auto p = file.AssetHeader.relativeAssetPath.string();
		std::cout << "Pressed: " << p.c_str() << std::endl;
	});

	auto textureSheetFileBrowser = new FileBrowser(Strings::Directory_TextureSheets, "TextureSheets", onTexSheetEdit, nullptr, onTexSheetEdit);
	fileBrowsers = { tileFileBrowser, spriteFileBrowser, textureSheetFileBrowser };

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

	SDLWindow = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
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
	SetWindowsDPIScaleAware();


	return true;
}
void Rendering::MainWindow::LoadLevel(Level* level) {
	if (loadedLevel != nullptr) UnloadLevel();
	loadedLevel = level;
	level->TileMapManagerUPtr->gridToolBar = gridToolBar;
	Renderer::RenderObjects.push_back(level->TileMapManagerUPtr.get());

	if (level->TileMapManagerUPtr->tileMaps.size() > 0) {
		level->TileMapManagerUPtr->SetActiveTileMap(level->TileMapManagerUPtr->tileMaps[0]);
	}
	SetWindowDirtyFlag(false);
	SetWindowTitle(level->Name);
}
void Rendering::MainWindow::UnloadLevel() {
	if (loadedLevel == nullptr) return;
	Rendering::Renderable* tileMapManager = static_cast<Rendering::Renderable*>(loadedLevel->TileMapManagerUPtr.get());
	auto removeIt = std::remove(Renderer::RenderObjects.begin(), Renderer::RenderObjects.end(), tileMapManager);
	Renderer::RenderObjects.erase(removeIt);


	delete loadedLevel;
}
void Rendering::MainWindow::SaveCurrentLevel(std::string nameOverride) {
	if (!nameOverride.empty()) loadedLevel->Name = nameOverride;
	loadedLevel->SaveToFile();
	SetWindowDirtyFlag(false);
	SetWindowTitle(loadedLevel->Name);
}

void MainWindow::RefreshFileBrowserDirectories() {
	for (const auto& fBrowser : fileBrowsers) fBrowser->RefreshCurrentDirectory();
}

void MainWindow::Render() {
	// clear color, depth and stencil buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Order of Render OpenGL and ImGui does not seem to matter?
	Renderer::Render();
	RenderImGui();

	SDL_GL_SwapWindow(SDLWindow);
}

void Rendering::MainWindow::SetWindowDirtyFlag(bool dirty) {
	bool wasAlreadyDirty = loadedLevel->isDirty == dirty;
	loadedLevel->isDirty = dirty;

	if (!wasAlreadyDirty) SetWindowTitle();
}
void Rendering::MainWindow::SetWindowTitle(std::string title) {
	std::string newTitle = windowTitle;
	if (!title.empty()) {
		windowTitle = "LevelEditor: " + title;
		newTitle = windowTitle;
	}
	if (loadedLevel && loadedLevel->isDirty) newTitle += "*";
	SDL_SetWindowTitle(SDLWindow, newTitle.c_str());
}

void MainWindow::RenderImGui() {
	// Required before ImGui logic
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(SDLWindow);
	using namespace ImGui;
	ImGui::NewFrame();

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	// ########################## ImGui logic below this line ##############################

	static bool showDebugWindow = false;
	if (showDebugWindow) {
		ImGui::ShowDemoWindow();
	}

	// Main Cam Controls
	Camera::Main->DearImGuiWindow();

	static bool saveLevelDialogue = false;

	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S)) {
		if (!loadedLevel->Name.empty() && loadedLevel->Name != "untitled") SaveCurrentLevel();
		else saveLevelDialogue = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
		RefreshFileBrowserDirectories();
	}

	// Menu Bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New...")) {
				//open dialogue asking for name
				Level* level = Level::CreateDefaultLevel();
				LoadLevel(level);
			}
			if (ImGui::MenuItem("Open Level...")) {
				std::string absolutePath;
				//constexpr char filter[] = "Image Files (JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM)\0*.jpeg;*.png;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic;*.pnm\0\0";
				constexpr char filter[] = "Level File (.level)\0*.level\0\0";
				if (Files::OpenFileDialog(absolutePath, filter)) {
					Level* level = nullptr;
					if (Level::LoadFromFile(Files::GetRelativePath(absolutePath).c_str(), level)) {
						LoadLevel(level);
					}
				}
			}
			if (ImGui::MenuItem("Save", "CTRL + S")) {
				std::string errorMsg;
				if (!loadedLevel->CanSave(errorMsg)) {
					saveLevelDialogue = true;
					std::cout << errorMsg << std::endl;
				}
				else loadedLevel->SaveToFile();
			}
			if (ImGui::MenuItem("Save as...")) {
				saveLevelDialogue = true;
			}
			ImGui::EndMenu();
		}

		if (saveLevelDialogue) {
			ImGuiHelper::CenterNextWindow(ImVec2(0, 0), ImGuiCond_Appearing);
			if (Begin("Saving - Enter a name for current Level:", &saveLevelDialogue, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
				std::string errorMessage;
				InputTextWithHint("Name", "cool_level_69", &loadedLevel->Name);
				bool hasError = !loadedLevel->CanSave(errorMessage, false);
				if (hasError) {
					ImGui::TextUnformatted(errorMessage.c_str());
				}

				if (hasError) BeginDisabled();
				bool savePressed = Button("Save");
				if (hasError) EndDisabled();
				SameLine();
				bool quitPressed = Button("Cancel");

				if (savePressed || quitPressed) {
					if (savePressed) {
						SaveCurrentLevel("");
					}
					//Reset everything
					saveLevelDialogue = false;
				}
			}
			End();
		}
		static bool showTextureDebugViewer = false;

#ifdef _DEBUG
		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Show Demo Window", 0, showDebugWindow)) {
				showDebugWindow = !showDebugWindow;
			}
			if (ImGui::MenuItem("Show TextureDebugViewer", nullptr, showTextureDebugViewer)) {
				showTextureDebugViewer = !showTextureDebugViewer;
			}
			if (MenuItem("Recompile Shader")) {
				Renderer::CompileShader();
			}
			if (MenuItem("test GUID")) {
				auto assetid1 = AssetId::CreateNewAssetId();
				auto assetid2 = AssetId::CreateNewAssetId();

				bool same1 = assetid1 == assetid2;

				AssetId assetid1Copy = assetid1;

				bool same2 = assetid1 == assetid1Copy;

				std::string assetId1Str = assetid1;

				AssetId assetId1FromString;
				bool parseSuccess = AssetId::TryParse(assetId1Str, assetId1FromString);
				bool same3 = assetid1 == assetId1FromString;


				printf("AssetId1: %s, AssetId2: %s, same? %s\n", assetid1.ToString().c_str(), assetid2.ToString().c_str(), same1 ? "Yes" : "No");
				printf("AssetId1: %s, AssetId1Copy: %s, same? %s\n", assetid1.ToString().c_str(), assetid1Copy.ToString().c_str(), same2 ? "Yes" : "No");
				if (parseSuccess)
					printf("AssetId1: %s, assetId1FromString: %s, same? %s\n", assetid1.ToString().c_str(), assetId1FromString.ToString().c_str(), same3 ? "Yes" : "No");
				else
					printf("Parse failed.");
			}
			ImGui::EndMenu();
		}
#endif

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
			if (ImGui::MenuItem("Show Grid ", 0, Renderer::DrawGrid)) {
				Renderer::DrawGrid = !Renderer::DrawGrid;
			}
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
		if(ImGui::MenuItem("Refresh Directories", "F5")) {
			RefreshFileBrowserDirectories();
		}

		ImGui::EndMainMenuBar();

	}

	// Grid Tool Window
	static bool toolWindowOpen = true;
	constexpr ImGuiWindowFlags toolFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
	if (Begin("Tools", &toolWindowOpen, toolFlags)) {
		int toolCount = 3;
		auto buttonSize = ImVec2(32, 32);
		const char* iconStrings[] = { Strings::Icon_Tool_Place, Strings::Icon_Tool_Erase, Strings::Icon_Tool_Select };
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

			if (ImageButton(reinterpret_cast<ImTextureID*>(textureID), buttonSize, ImVec2(0, 1), ImVec2(1, 0), framePadding)) {
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

	loadedLevel->TileMapManagerUPtr->RenderImGuiWindow();
	for (auto& fBrowser : fileBrowsers)  fBrowser->RenderImGuiWindow();

	FileEditWindow::RenderAll();

	// ############################### ImGui logic above this line #################################
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
	for (auto& fBrowser : fileBrowsers) delete fBrowser;
	Renderer::Exit();
	SDL_DestroyWindow(SDLWindow);
	UnloadLevel();
	delete gridToolBar;
	SDLWindow = nullptr;
}
