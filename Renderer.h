#pragma once
#include <SDL_video.h>
#include <vector>

namespace Rendering {
	class Shader;
	class MainWindow;
	class Renderable;
	class Camera;

	class Renderer {
		inline static Camera* camera = nullptr;

		static bool InitOpenGL(SDL_Window* window);
	public:
		inline static Shader* defaultShader = nullptr;
		inline static Shader* gridShader = nullptr;

		inline static bool DrawGrid = true;

		inline static SDL_GLContext gl_context = nullptr;
		inline static std::vector<Renderable*> RenderObjects;
		static bool Init();
		static void Render();
		static void CompileShader();

		static void Exit();
	};
}

