#pragma once
#include <SDL_video.h>

#include "MainWindow.h"
#include "Shader.h"

namespace Rendering {
	class Renderer {
		inline static MainWindow* mainWindow;
		inline static Camera* camera = nullptr;

		static bool InitOpenGL(SDL_Window* window);
	public:
		inline static Shader* defaultShader = nullptr;
		inline static Shader* gridShader = nullptr;


		inline static SDL_GLContext gl_context = nullptr;
		inline static std::vector<Renderable*> RenderObjects;
		static bool Init(MainWindow* mainWindow);
		static void Render();
		static void CompileShader();

		static void Exit() {
			delete defaultShader;
			delete gridShader;
			delete camera;
		}
	};
}

