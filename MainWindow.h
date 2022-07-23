#pragma once
#include <SDL_video.h>
#include <vector>

class InputMouseEvent;
struct InputMouseBinding;

namespace Tiles {
	class TileMap;
}

namespace GridTools {
	class GridToolBar;
}

namespace Rendering {
	
class MainWindow {
	inline static SDL_Window* SDLWindow = nullptr;

	static int width;
	static int height;
	const char* m_title = "New Window";
	short renderMode = 0;

	InputMouseBinding* binding = nullptr;

	GridTools::GridToolBar* gridToolBar = nullptr;
	std::vector<Tiles::TileMap*> tileMaps;


	void RenderImGui();
	bool InitSDL();
	bool InitDearImGui();

public:
	static SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int new_width, int new_height, const char* title);
	void OnMouseInput(const InputMouseEvent* event);
	bool Initialize();
	void Render();


	static void OnResized(int width, int height);
	void Close();

	static void GetSize(int& out_width, int& out_height) {
		out_width = width;
		out_height = height;
	}
};
}
