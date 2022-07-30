#pragma once
#include <SDL_video.h>
#include <string>

class InputMouseEvent;
struct InputMouseBinding;
class Level;

namespace GridTools {
	class GridToolBar;
}

namespace Rendering {
	
class MainWindow {
	inline static SDL_Window* SDLWindow = nullptr;

	static int width;
	static int height;
	std::string windowTitle;
	short renderMode = 0;

	InputMouseBinding* binding = nullptr;

	Level* loadedLevel = nullptr;

	GridTools::GridToolBar* gridToolBar = nullptr;

	void RenderImGui();
	bool InitSDL();
	bool InitDearImGui();

	void LoadLevel(Level* level);
	void UnloadLevel();
	void SaveCurrentLevel(std::string nameOverride = "");

public:
	static SDL_Window* GetSDLWindow() { return SDLWindow; }
	MainWindow(int new_width, int new_height, const char* title);
	void OnMouseInput(const InputMouseEvent* event);
	bool Initialize();
	void Render();

	void SetWindowDirtyFlag(bool dirty);
	void SetWindowTitle(std::string title = "");


	static void OnResized(int width, int height);
	void Close();

	static void GetSize(int& out_width, int& out_height) {
		out_width = width;
		out_height = height;
	}
};
}
