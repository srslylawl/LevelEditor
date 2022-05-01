#pragma once
#include <SDL_video.h>

#include "MainWindow.h"
#include "Shader.h"

class Renderer {
	inline static MainWindow* mainWindow;
	inline static Shader* defaultShader = nullptr;
	inline static Shader* gridShader = nullptr;
	inline static Camera* camera = nullptr;

	static bool InitOpenGL(SDL_Window* window);
public:
	inline static SDL_GLContext gl_context = nullptr;
	static bool Init(MainWindow* mainWindow);
	static void Render();

	static void Exit() {
		delete defaultShader;
		delete camera;
	}
};

