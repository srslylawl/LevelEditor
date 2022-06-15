#pragma once

#include "Camera.h"
#include "GridToolBar.h"
#include "Mesh.h"
#include "TileMap.h"

namespace Rendering {
	
class MainWindow {
	SDL_Window* SDLWindow = nullptr;

	static int width;
	static int height;
	const char* m_title = "New Window";

	bool showDebugWindow = false;

	short renderMode = 0;

	std::vector<Mesh::StaticMesh> meshes;

	InputMouseBinding* binding = nullptr;

	void RenderImGui();
	bool InitSDL();
	bool InitDearImGui();

	GridTools::GridToolBar* gridToolBar = nullptr;

	ivec2 previousMouseGridPos = {INT_MAX, INT_MAX};


public:
	SDL_Window* GetSDLWindow() const { return SDLWindow; }
	MainWindow(int width, int height, const char* title);
	void OnMouseInput(const InputMouseEvent* event);
	bool Initialize();
	void Render();

	Tiles::TileMap* tileMap = nullptr;
	
	void OnResized(int width, int height);
	void Close();

	static void GetSize(int& out_width, int& out_height) {
		out_width = width;
		out_height = height;
	}
};
}
