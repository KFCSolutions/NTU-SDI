
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <SDL.h>
#include <imfilebrowser.h>
#include <algorithm>
#include <GL/gl3w.h>  
#include <iostream>
#include <sstream>
#include <string>
#include <thread>  
#include <fstream>
#include <chrono>
#define STB_IMAGE_IMPLEMENTATION
#include <stb-image.h>
#include "KFCLabel.h"

std::string curImg = "N/A";
std::string curTag = "N/A";
std::string curLabel = "N/A";
std::string currentShape = "N/A";

bool isImageModuleAlreadyOpen = false;
bool isTextureAlreadyLoaded = false;
int item_current = 0;
int my_image_width = 0;
int my_image_height = 0;
GLuint my_image_texture = 0;
float polygonSize = 0;
int polygonEdges = 0;
std::vector<std::string> images = {  };
std::vector<std::string> classes = {  };
ImGui::FileBrowser addImageDialog;
ImGui::FileBrowser classFileDialog;
ImGui::FileBrowser openProjectDialog;
std::vector<KFCLabel> KFCLabels;
char fileName[512];
std::string projectLoadData;
using json = nlohmann::json;


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


void drawShapes(ImVec2 canvas_pos, ImVec2 canvas_size, int labelID) {
	if (KFCLabels[labelID].getShape() == "Triangle") {
		ImGui::GetOverlayDrawList()->AddTriangle(
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y),
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition2().x, canvas_pos.y + KFCLabels[labelID].getMousePosition2().y),
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition3().x, canvas_pos.y + KFCLabels[labelID].getMousePosition3().y), IM_COL32(0, 255, 0, 200), 5);

		ImGui::GetWindowDrawList()->AddText(ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition2().x, canvas_pos.y + KFCLabels[labelID].getMousePosition2().y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), KFCLabels[labelID].getClassName().c_str());
	}

	if (KFCLabels[labelID].getShape() == "Rectangle") {
		ImGui::GetOverlayDrawList()->AddRect(
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y),
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition2().x, canvas_pos.y + KFCLabels[labelID].getMousePosition2().y),
			IM_COL32(0, 255, 0, 200),
			0,
			ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight,
			5);
		ImGui::GetWindowDrawList()->AddText(ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), KFCLabels[labelID].getClassName().c_str());
	}

	if (KFCLabels[labelID].getShape() == "Trapezium") {
		ImGui::GetOverlayDrawList()->AddQuad(
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y),
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition2().x, canvas_pos.y + KFCLabels[labelID].getMousePosition2().y),
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition3().x, canvas_pos.y + KFCLabels[labelID].getMousePosition3().y),
			ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition4().x, canvas_pos.y + KFCLabels[labelID].getMousePosition4().y),
			IM_COL32(0, 255, 0, 200), 5);
		ImGui::GetWindowDrawList()->AddText(ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), KFCLabels[labelID].getClassName().c_str());
	}

	if (KFCLabels[labelID].getShape() == "Polygon") {
		float sz = 76.0f;
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y), KFCLabels[labelID].getPolySize() * 0.5f, IM_COL32(0, 255, 0, 200), KFCLabels[labelID].getEdges(), 5);
		ImGui::GetWindowDrawList()->AddText(ImVec2(canvas_pos.x + KFCLabels[labelID].getMousePosition1().x + KFCLabels[labelID].getPolySize() * 0.5f, canvas_pos.y + KFCLabels[labelID].getMousePosition1().y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), KFCLabels[labelID].getClassName().c_str());
	}

	if (KFCLabels[labelID].getName() == curLabel) {
		KFCLabels[labelID].setPolySize(polygonSize);
		KFCLabels[labelID].setEdges(polygonEdges);
		if (GetKeyState(0x70) & 0x8000)
			KFCLabels[labelID].setMousePosition1(ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y));
		if (GetKeyState(0x71) & 0x8000)
			KFCLabels[labelID].setMousePosition2(ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y));
		if (GetKeyState(0x72) & 0x8000)
			KFCLabels[labelID].setMousePosition3(ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y));
		if (GetKeyState(0x73) & 0x8000)
			KFCLabels[labelID].setMousePosition4(ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y));
	}
}



void drawLabels(ImVec2 canvas_pos, ImVec2 canvas_size) {
	for (int i = 0; i < KFCLabels.size(); i++) {
		if (KFCLabels[i].getFilePath() == curImg) {
			drawShapes(canvas_pos, canvas_size, i );			
		}
	}
}

void loadImageModule(std::string img) {
	ImGui::Begin(img.c_str());
	ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
	ImVec2 canvas_size = ImGui::GetContentRegionAvail();
	if (isTextureAlreadyLoaded) {
		bool ret = loadTextureFromFile(img.c_str(), &my_image_texture, &my_image_width, &my_image_height);
		isTextureAlreadyLoaded = false;
	}
	ImGui::Text("Image Size = %d x %d", my_image_width, my_image_height);
	ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(my_image_width, my_image_height));
	drawLabels(canvas_pos, canvas_size);

	ImGui::End();
}

void clearAll() {
	images = {};
	classes = {};
	KFCLabels = {};
}

void loadSaveFile() {
	clearAll();
	std::ifstream i(projectLoadData);
	std::string getFileName = projectLoadData.substr(projectLoadData.find_last_of("\\") + 1);
	strcpy(fileName, getFileName.c_str()); // Imagine using a C library
	json j = json::parse(i);
	images = j["images"].get<std::vector<std::string>>();
	classes = j["classes"].get<std::vector<std::string>>();

	for (json x : j["labels"]) {
		KFCLabel kfcLabel("l");
		KFCLabels.push_back(kfcLabel);
		int labelID = KFCLabels.size() - 1;
		KFCLabels[labelID].setClassName(x["classname"]);
		KFCLabels[labelID].setEdges(x["edges"]);
		KFCLabels[labelID].setFilePath(x["image"]);
		KFCLabels[labelID].setName(x["name"]);
		KFCLabels[labelID].setPolySize(x["polysize"]);
		KFCLabels[labelID].setShape(x["shape"]);
		KFCLabels[labelID].setMousePosition1(ImVec2(x["pos1"][0], x["pos1"][1]));
		KFCLabels[labelID].setMousePosition2(ImVec2(x["pos2"][0], x["pos2"][1]));
		KFCLabels[labelID].setMousePosition3(ImVec2(x["pos3"][0], x["pos3"][1]));
		KFCLabels[labelID].setMousePosition4(ImVec2(x["pos4"][0], x["pos4"][1]));
	}
}


void setUpDialogs() {

	//Main Menu Browser dialogs init
	addImageDialog.SetTitle("Add new Image");


	classFileDialog.SetTitle("Select Class File");
	classFileDialog.SetTypeFilters({ ".class" });

	openProjectDialog.SetTitle("Select Project File");
	openProjectDialog.SetTypeFilters({ ".json"});

	addImageDialog.Display();
	classFileDialog.Display();
	openProjectDialog.Display();

	// Do the following when a file is selected from the file browser
	if (openProjectDialog.HasSelected())
	{
		projectLoadData = openProjectDialog.GetSelected().string();
		openProjectDialog.ClearSelected();
		loadSaveFile();
	}

	if (addImageDialog.HasSelected())
	{
		if (addImageDialog.GetSelected().string().find(".png") != std::string::npos || addImageDialog.GetSelected().string().find(".jpg") != std::string::npos) {
			images.push_back(addImageDialog.GetSelected().string());
		}
		else {
			std::cout << "Failed to add image, must be type JPG or PNG" << std::endl;
		}
		addImageDialog.ClearSelected();

	}

	if (classFileDialog.HasSelected())
	{
		classes = { };
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

void saveProject() {
	json j;
	j["images"] = images;
	j["classes"] = classes;
	int i = 0;
	for (KFCLabel label : KFCLabels) {
		j["labels"][i]["image"] = label.getFilePath();
		j["labels"][i]["classname"] = label.getClassName();
		j["labels"][i]["shape"] = label.getShape();
		j["labels"][i]["name"] = label.getName();
		j["labels"][i]["polysize"] = label.getPolySize();
		j["labels"][i]["edges"] = label.getEdges();
		j["labels"][i]["pos1"] = { label.getMousePosition1().x, label.getMousePosition1().y };
		j["labels"][i]["pos2"] = { label.getMousePosition2().x, label.getMousePosition2().y };
		j["labels"][i]["pos3"] = { label.getMousePosition3().x, label.getMousePosition3().y };
		j["labels"][i]["pos4"] = { label.getMousePosition4().x, label.getMousePosition4().y };
		i++;
	}

	std::string s = j.dump();
	std::string x(fileName);
	if (x.find(".json") != std::string::npos) {
		std::ofstream o(fileName);
		o << std::setw(4) << s << std::endl;
	}
	else {
		std::ofstream o(x + ".json");
		o << std::setw(4) << s << std::endl;
	}

}

void autoSave() {
	using namespace std::chrono_literals;
	while (true) {
		using namespace std::chrono_literals;
		auto start = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(30s);
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed = end - start;
		if (fileName[0] == '\0') {
			std::cout << "Auto Save Disabled. Please set the project name!\n";
		}
		else {
			saveProject();
			std::cout << "Auto Saved Project " << elapsed.count() << " ms\n";
		}
	}
}



void initMainMenu() {
	//Starts drawing the menu dialog
	ImGui::Begin("NTU Programming Assignment"); // Sets the title of the dialog


	if (ImGui::CollapsingHeader("File Management")) {
		if (ImGui::Button("Open Annotation File")) {
			openProjectDialog.Open();
		}
		ImGui::InputText("File Name", fileName, IM_ARRAYSIZE(fileName));
		ImGui::SameLine();
		if (ImGui::Button("Save Project")) {
			saveProject();
		}
	}

	if (ImGui::CollapsingHeader("Images")) {
		if (ImGui::Button("Add Image"))
			addImageDialog.Open();
		ImGui::SameLine();
		ImGui::Text("%d Images Loaded", images.size());
		if (ImGui::Button("Ascending Order")) {
			std::sort(images.begin(), images.end());
		}
		ImGui::SameLine();
		if (ImGui::Button("Decending Order")) {
			std::sort(images.rbegin(), images.rend());
		}
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::TreeNode("Loaded Images")) {
			for (auto imageLocation : images) {
				std::string fileName = imageLocation.substr(imageLocation.find_last_of("\\") + 1);
				if (ImGui::Button(fileName.c_str())) {
					curImg = imageLocation;
					std::cout << "Setting Current Image to :" << curImg << std::endl;
					isImageModuleAlreadyOpen = true;
					isTextureAlreadyLoaded = true;
					curLabel = "N/A";
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
		if (ImGui::Button("Ascending Order ")) {
			std::sort(classes.begin(), classes.end());
		}
		ImGui::SameLine();
		if (ImGui::Button("Decending Order ")) {
			std::sort(classes.rbegin(), classes.rend());
		}
		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::TreeNode("Loaded Classes")) {
			for (auto value : classes) {
				if (ImGui::Button(value.c_str())) {
					curTag = value.c_str();
					std::cout << "Setting Current Class Tag :" << curTag << std::endl;
				}
			}
			ImGui::TreePop();
		}
	}


	if (ImGui::CollapsingHeader("Annotations")) {
		ImGui::Text("Currently selected label: %s", curLabel.c_str());
		const char* items[] = { "Triangle", "Trapezium", "Rectangle", "Polygon" };
		ImGui::Combo("Select Shape", &item_current, items, IM_ARRAYSIZE(items));
		if (ImGui::Button("New Label")) {
			if (curTag.find("N/A") == std::string::npos) {
				KFCLabel kfcLabel("l");
				KFCLabels.push_back(kfcLabel);
				int labelID = KFCLabels.size() - 1;
				KFCLabels[labelID].setClassName(curTag);
				KFCLabels[labelID].setName(KFCLabels[labelID].getName() + std::to_string(labelID));
				switch (item_current) {
				case 0:
					KFCLabels[labelID].setShape("Triangle");
					break;
				case 1:
					KFCLabels[labelID].setShape("Trapezium");
					break;
				case 2:
					KFCLabels[labelID].setShape("Rectangle");
					break;
				case 3:
					KFCLabels[labelID].setShape("Polygon");
					break;
				}
				KFCLabels[labelID].setFilePath(curImg);
			}
			else {
				std::cout << "Please select a class before creating a label!" << std::endl;
			}

		}
		ImGuiIO& io = ImGui::GetIO();
		std::string title = "List of labels for image: " + curImg;
		if (currentShape.find("Polygon") != std::string::npos) {
			if (ImGui::TreeNode("Polygon Settings")) {

				ImGui::DragFloat("Size", &polygonSize, 30.2f, 31.0f, 200.0f, "%.0f");
				ImGui::DragInt("Edges", &polygonEdges, 1, 0, 8);
				ImGui::TreePop();
			}
		}
		if (ImGui::TreeNode(title.c_str())) {
			for (KFCLabel label : KFCLabels) {
				if (label.getFilePath() == curImg) {
					std::string name = label.getName() + " #" +label.getClassName() + " [" + label.getShape() + "]";
					if (ImGui::Button(name.c_str())) {
						curLabel = label.getName();
						currentShape = label.getShape();
						polygonSize = label.getPolySize();
						polygonEdges = label.getEdges();
					}
				}
			}
			ImGui::TreePop();
		}
	}


	if (ImGui::CollapsingHeader("Information")) {
		ImGui::Text("KEYBINDING FOR POINTS");
		ImGui::BulletText("POINT 1: F1");
		ImGui::BulletText("POINT 2: F2");
		ImGui::BulletText("POINT 3: F3");
		ImGui::BulletText("POINT 4: F4");
	}
	ImGui::End();	
	setUpDialogs();
	}

// Entry Point -- This is what gets executed first
int main(int, char**)
{
	// Runs some basic SDL setup
	guiInit();
    const char* glsl_version = "#version 130";
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Known For Code Solutions Image Labeler - NTU Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); 
	// Setting up OpenGL gl3w 
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


    ImVec4 backgroundColor = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);

	//Start Save Thread 
	std::thread first(autoSave);

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


		if (isImageModuleAlreadyOpen)
			loadImageModule(curImg);

		// Calls the function to render the UI
		initMainMenu();
	
        // Rendering
        ImGui::Render();
		// This sets up the background and sets its colour
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
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
