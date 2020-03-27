
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb-image.h>
#include <imfilebrowser.h>
#include <algorithm>
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

ImVec2 mousePosInCanvas1;
ImVec2 mousePosInCanvas2;
ImVec2 mousePosInCanvas3;

static std::string curImg = "KFC.jpg";
static std::string curTag = "N/A";
static bool showImage = false;
std::vector<std::string> images = {  };
std::vector<std::string> classes = {  };
ImGui::FileBrowser addImageDialog;
ImGui::FileBrowser classFileDialog;

bool loadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

// IMGUI init lines and arguments
int guiInit() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

static void loadImage(std::string img) {
	int my_image_width = 0;
	int my_image_height = 0;
	GLuint my_image_texture = 0;
	bool ret = loadTextureFromFile(img.c_str(), &my_image_texture, &my_image_width, &my_image_height);
	ImGui::Begin(img.c_str());
	ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); 
	ImVec2 canvas_size = ImGui::GetContentRegionAvail();    
	ImGui::Text("pointer = %p", my_image_texture);
	ImGui::Text("size = %d x %d", my_image_width, my_image_height);
	ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));

	//TODO: this is just demo code for drawing images and a prototype. Need to fully rework this
	std::cout << mousePosInCanvas1.x << ": " << mousePosInCanvas1.y << std::endl;
	std::cout <<mousePosInCanvas2.x << ": " <<mousePosInCanvas2.y << std::endl;
	std::cout <<mousePosInCanvas3.x << ": " <<mousePosInCanvas3.y << std::endl;
	ImGui::GetOverlayDrawList()->AddTriangle(ImVec2(canvas_pos.x + mousePosInCanvas1.x, canvas_pos.y + mousePosInCanvas1.y), ImVec2(canvas_pos.x +mousePosInCanvas2.x, canvas_pos.y +mousePosInCanvas2.y), ImVec2(canvas_pos.x +mousePosInCanvas3.x, canvas_pos.y +mousePosInCanvas3.y), IM_COL32(0, 255, 0, 200), 5);

	if (GetKeyState('Q') & 0x8000)
	{
		mousePosInCanvas1 = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
	}
	if (GetKeyState('W') & 0x8000)
	{
		mousePosInCanvas2 = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
	}
	if (GetKeyState('E') & 0x8000)
	{
		mousePosInCanvas3 = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
	}

	ImGui::End();
}

static void initMainMenu() {
	bool show_shape = false;

	//Main Menu Browser dialogs init
	addImageDialog.SetTitle("Add new Image");
	addImageDialog.SetTypeFilters({ ".png", ".jpg" });

	classFileDialog.SetTitle("Select Class File");
	classFileDialog.SetTypeFilters({ ".class", });

	// Main Menu init. All this draws the main menu
	ImGui::Begin("NTU Programming Assignment - By KFC Solutions"); 
	if (ImGui::CollapsingHeader("Images")) {
		if (ImGui::Button("Add Image"))
			addImageDialog.Open();
		ImGui::SameLine();
		ImGui::Text("%d Images Loaded", images.size());
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::TreeNode("Loaded Images")) {

			for (auto value : images) {
				if (ImGui::Button(value.c_str())) {
					curImg = value;
					std::cout << "Setting Current Image to :" << curImg << std::endl;
					showImage = true;
				}

			}
			ImGui::TreePop();
		}
	}
	if (ImGui::CollapsingHeader("Classes")) {
		if (ImGui::Button("Pick Class File"))
			classFileDialog.Open();
		ImGui::SameLine();
		ImGui::Text("%d Classes Loaded", classes.size());
		ImGui::Text("Currently selected tag: %s", curTag.c_str());
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::TreeNode("Loaded Classes")) {

			for (auto value : classes) {
				if (ImGui::Button(value.c_str())) {
					curTag = value;
					std::cout << "Setting Current Class Tag :" << curTag << std::endl;
				}

			}
			ImGui::TreePop();
		}

	}

	ImGui::End();

	addImageDialog.Display();
	classFileDialog.Display();

	// Do the following when a file is selected from the file browse 
	if (addImageDialog.HasSelected())
	{
		//std::cout << "Selected filename" << addImageDialog.GetSelected().string() << std::endl;
		images.push_back(addImageDialog.GetSelected().string());
		addImageDialog.ClearSelected();
	}

	if (classFileDialog.HasSelected())
	{
		std::string line;
		std::cout << "Selected filename" << classFileDialog.GetSelected().string() << std::endl;
		std::ifstream file(classFileDialog.GetSelected().string());
		while (std::getline(file, line))
		{
			classes.push_back(line);
		}
		classFileDialog.ClearSelected();
	}

}


// Main code
int main(int, char**)
{
	guiInit();
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Known For Code Solutions Image Labeler - NTU Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); 

    bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);


    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		// USED FOR DEBUGGING IMGUI ImGui::ShowDemoWindow();
		if (showImage)
			loadImage(curImg);
		initMainMenu();
		
	
        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
