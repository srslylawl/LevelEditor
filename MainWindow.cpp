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
#include "Shader.h"
#include "Time.h"

using namespace std;

MainWindow::MainWindow(int width, int height, const char* title) : m_width(width), m_height(height), m_title(title), elementBufferObject(0) {}

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

void GenTexture(const char* path, MainWindow* mainWindow) {
	unsigned int texture;
	glGenTextures(1, &texture);

	int width, height, channelCount;
	unsigned char* imageData = nullptr;
	if (!LoadImage(path, imageData, &width, &height, &channelCount)) {
		cout << "Unable to load image: " << path << " : " << stbi_failure_reason() << endl;
		return;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int colorProfile = 0;
	switch (channelCount) {
	case 3:
		colorProfile = GL_RGB;
		break;
	case 4:
		colorProfile = GL_RGBA;
		break;
	default:
		break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, colorProfile, width, height, 0, colorProfile, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	mainWindow->currentTexture = texture;

	cout << "Image bound to textureID: " << texture << "Channels: " << channelCount << endl;

	stbi_image_free(imageData);
}

bool MainWindow::Initialize() {
	if (!InitSDL()) return false;
	if (!InitOpenGL()) return false;
	if (!InitDearImGui()) return false;

	//update while resizing - does not work though, according to google its a backend limitation?
	SDL_AddEventWatch(WindowResizeEvent, this);
	return true;
}
bool MainWindow::InitSDL() {
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

	return true;
}
bool MainWindow::InitOpenGL() {
	// set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE
	);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

	gl_context = SDL_GL_CreateContext(SDLWindow);
	if (!gl_context) {
		std::cout << "Unable to create gl context " << SDL_GetError() << std::endl;
		return false;
	}
	SDL_GL_MakeCurrent(SDLWindow, gl_context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
		return false;
	}

	// set clearing color (background color)
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	// __vertex input__
	// create vertex array object (VAO) and bind it
	glGenVertexArrays(1, &VertexArrayObject);
	glBindVertexArray(VertexArrayObject);

	// create vertex buffer object (VBO) and bind it, can only generate one of each type
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// create element buffer object (EBO) and bind it -> allows reusing of verts, binds it to VAO as well
	glGenBuffers(1, &elementBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);

	// left side are verts, right side are tex coords
	const float quadVerts[] = {
	 0.5f,  0.5f, 0.0f,		1.0f, 1.0f, // top right
	 0.5f, -0.5f, 0.0f,		1.0f, 0.0f, // top left
	-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, // bottom left
	-0.5f,  0.5f, 0.0f,		0.0f, 1.0f, // bottom right 
	};

	const unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
	};

	//assign buffer data to EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// feed verts into array buffer -- since only one buffer was created, it picks that one
	// GL_STATIC_DRAW since we never change the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	// specify how to interpret vertex data, in this case vertex position
	// 1. 0 = layout, specified in vert shader -> in this case refers to ver pos
	// 2. 3 = size of vertex attribute, 3 since its a vector3 for position
	// 3. GL_FLOAT = type
	// 4. GL_FALSE = dont normalize (used for booleans)
	// 5. STRIDE, aka space between vertex attributes, counting from first, since data is 3x float pos and 2x float texcoord -> stride = 5*float size
	// 6. (void*)0 is a nullptr?
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	// inform gl about tex coords
	// 1. 1 = layout of tex coord in vert shader -> 2
	// 2. 2d vector, so size of 2
	// 3-5. same as above
	// 6. offset, since texcoords are right after the vec3 floats, its 3*sizeof float, cast to a void type pointer
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//init shaders
	const filesystem::path vertShaderPath = filesystem::current_path().append("Shaders/defaultVertShader.vert");
	const filesystem::path fragShaderPath = filesystem::current_path().append("Shaders/defaultFragShader.frag");

	shaderProgramUPTR = make_unique<Shader>(vertShaderPath.string().c_str(), fragShaderPath.string().c_str());

	return true;
}
bool MainWindow::InitDearImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// does not seem to do anything rn, so commented out
	//ImGuiIO& io = ImGui::GetIO();
	//(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(SDLWindow, gl_context);
	const std::string glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

	return true;
}

void MainWindow::Render() {
	// clear color, depth and stencil buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Order of Render OpenGL and ImGui does not seem to matter?
	RenderOpenGL();
	MainWindow::RenderImGui();

	SDL_GL_SwapWindow(MainWindow::SDLWindow);
}

void MainWindow::RenderOpenGL() {
	//___ LOOPED RENDERING CODE
	// use shader program

	float greenValue = (sin(Time::GetTime()) / 2.0f) + 0.5f;
	int vertexColorLocation = glGetUniformLocation(shaderProgramUPTR->ID, "ourColor");
	shaderProgramUPTR->use();
	glUniform1i(glGetUniformLocation(shaderProgramUPTR->ID, "texture1"), 0); // TODO: dont set this in loop!
	glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentTexture);
	// use VAO
	// EBO is already bound to VAO so it gets bound automatically
	glBindVertexArray(VertexArrayObject);

	// draw triangles using EBO, takes from bound element array buffer
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//unbind vertex array
	glBindVertexArray(0);
}
void MainWindow::RenderImGui() {
	// Required before ImGui logic
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(MainWindow::SDLWindow);
	ImGui::NewFrame();

	// ImGui logic here

	if (showDebugWindow) {
		ImGui::ShowDemoWindow();
	}
	//return;

	// Menu Bar (Top of Window thing)
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				//Do something
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::MenuItem("Show Demo Window", 0, showDebugWindow)) {
				showDebugWindow = !showDebugWindow;
			}
			if (ImGui::MenuItem("folderstuff")) {
				std::cout << "Items in Sprites: " << endl;
				if (VerifyDirectory("Sprites")) {
					for (const auto& entry : filesystem::directory_iterator(filesystem::current_path().append("Sprites")))
						std::cout << entry.path() << std::endl;
				}
			}
			if (ImGui::BeginMenu("Sprites")) {
				if (VerifyDirectory("Sprites")) {
					for (const auto& entry : filesystem::directory_iterator(filesystem::current_path().append("Sprites"))) {
						string item = entry.path().string();
						if (ImGui::MenuItem(item.c_str())) {
							cout << "Item clicked: " << item << endl;
							GenTexture(item.c_str(), this);
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


	// Required to render ImGuI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void MainWindow::OnResized(int width, int height) {
	m_height = height;
	m_width = width;
	glViewport(0, 0, m_width, m_height);
	//std::cout << "On Resized h: " << height << " w: " << width << endl;
}
void MainWindow::Close() {
	SDL_DestroyWindow(SDLWindow);
	SDLWindow = nullptr;
}
